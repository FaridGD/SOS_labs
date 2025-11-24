#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <errno.h>

struct fhdr {
    char nm[256];
    mode_t md;
    uid_t uid;
    gid_t gid;
    time_t mt;
    off_t sz;
};

ssize_t wr_all(int fd, const void* b, size_t cnt) {
    const char* p = (const char*)b;
    size_t left = cnt;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        left -= w;
        p += w;
    }
    return cnt;
}

ssize_t rd_all(int fd, void* b, size_t cnt) {
    char* p = (char*)b;
    size_t left = cnt;
    while (left > 0) {
        ssize_t r = read(fd, p, left);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return cnt - left;
        left -= r;
        p += r;
    }
    return cnt;
}

void show_help() {
    printf("Usage:\n");
    printf("./archiver arch_name -i file1\n");
    printf("./archiver arch_name -e file1\n"); 
    printf("./archiver arch_name -s\n");
    printf("./archiver -h\n");
}

int add_f(const char* arc, int cnt, char* f[]) {
    int afd = open(arc, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (afd < 0) return -1;
    
    char buf[65536];
    
    for (int i = 0; i < cnt; i++) {
        const char* fn = f[i];
        int ffd = open(fn, O_RDONLY);
        if (ffd < 0) { close(afd); return -1; }
        
        struct stat st;
        if (fstat(ffd, &st) != 0) { close(ffd); close(afd); return -1; }
        if (!S_ISREG(st.st_mode)) { close(ffd); continue; }
        
        struct fhdr h;
        memset(&h, 0, sizeof(h));
        strncpy(h.nm, fn, 255);
        h.md = st.st_mode;
        h.uid = st.st_uid;
        h.gid = st.st_gid;
        h.mt = st.st_mtime;
        h.sz = st.st_size;
        
        if (wr_all(afd, &h, sizeof(h)) != sizeof(h)) { close(ffd); close(afd); return -1; }
        
        off_t rem = h.sz;
        while (rem > 0) {
            ssize_t r = read(ffd, buf, rem > 65536 ? 65536 : rem);
            if (r < 0) { close(ffd); close(afd); return -1; }
            if (r == 0) break;
            if (wr_all(afd, buf, r) != r) { close(ffd); close(afd); return -1; }
            rem -= r;
        }
        close(ffd);
    }
    close(afd);
    return 0;
}

int lst_arc(const char* arc) {
    int afd = open(arc, O_RDONLY);
    if (afd < 0) return -1;
    
    struct fhdr h;
    while (1) {
        ssize_t r = rd_all(afd, &h, sizeof(h));
        if (r == 0) break;
        if (r != sizeof(h)) { close(afd); return -1; }
        
        printf("%s\t%ld bytes\tmode:%o\tuid:%u\tgid:%u\n",
               h.nm, h.sz, h.md, h.uid, h.gid);
        
        if (lseek(afd, h.sz, SEEK_CUR) == (off_t)-1) { close(afd); return -1; }
    }
    close(afd);
    return 0;
}

int in_lst(const char* n, int cnt, char* lst[]) {
    for (int i = 0; i < cnt; i++) {
        if (strcmp(n, lst[i]) == 0) return 1;
    }
    return 0;
}

int ext_rm(const char* arc, int cnt, char* f[]) {
    int afd = open(arc, O_RDONLY);
    if (afd < 0) return -1;
    
    char tmp[] = "/tmp/arcXXXXXX";
    int tfd = mkstemp(tmp);
    if (tfd < 0) { close(afd); return -1; }
    unlink(tmp);
    
    char buf[65536];
    int ret = 0;
    
    while (1) {
        struct fhdr h;
        ssize_t r = rd_all(afd, &h, sizeof(h));
        if (r == 0) break;
        if (r != sizeof(h)) { ret = -1; break; }
        
        int ext = in_lst(h.nm, cnt, f);
        if (ext) {
            int ofd = open(h.nm, O_WRONLY | O_CREAT | O_TRUNC, h.md & 07777);
            if (ofd < 0) { ret = -1; break; }
            
            off_t rem = h.sz;
            while (rem > 0) {
                ssize_t chunk = rem > 65536 ? 65536 : rem;
                ssize_t rr = rd_all(afd, buf, chunk);
                if (rr <= 0) { close(ofd); ret = -1; break; }
                if (wr_all(ofd, buf, rr) != rr) { close(ofd); ret = -1; break; }
                rem -= rr;
            }
            if (ret != 0) { close(ofd); break; }
            close(ofd);
            
            chown(h.nm, h.uid, h.gid);
            chmod(h.nm, h.md & 07777);
            struct utimbuf tb;
            tb.actime = h.mt;
            tb.modtime = h.mt;
            utime(h.nm, &tb);
        } else {
            if (wr_all(tfd, &h, sizeof(h)) != sizeof(h)) { ret = -1; break; }
            off_t rem = h.sz;
            while (rem > 0) {
                ssize_t chunk = rem > 65536 ? 65536 : rem;
                ssize_t rr = rd_all(afd, buf, chunk);
                if (rr <= 0) { ret = -1; break; }
                if (wr_all(tfd, buf, rr) != rr) { ret = -1; break; }
                rem -= rr;
            }
            if (ret != 0) break;
        }
    }
    
    if (ret == 0) {
        close(afd);
        int afd2 = open(arc, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (afd2 < 0) { close(tfd); return -1; }
        
        lseek(tfd, 0, SEEK_SET);
        while (1) {
            ssize_t rr = read(tfd, buf, 65536);
            if (rr < 0) {
                if (errno == EINTR) continue;
                close(tfd); close(afd2); return -1;
            }
            if (rr == 0) break;
            if (wr_all(afd2, buf, rr) != rr) { close(tfd); close(afd2); return -1; }
        }
        close(tfd);
        close(afd2);
    } else {
        close(afd);
        close(tfd);
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return 1;
    }
    
    if (strcmp(argv[1], "-h") == 0) {
        show_help();
        return 0;
    }
    
    if (argc < 3) {
        show_help();
        return 1;
    }
    
    const char* arc = argv[1];
    const char* cmd = argv[2];
    
    if (strcmp(cmd, "-i") == 0) {
        if (argc < 4) {
            show_help();
            return 1;
        }
        return add_f(arc, argc - 3, &argv[3]);
    } else if (strcmp(cmd, "-e") == 0) {
        if (argc < 4) {
            show_help();
            return 1;
        }
        return ext_rm(arc, argc - 3, &argv[3]);
    } else if (strcmp(cmd, "-s") == 0) {
        return lst_arc(arc);
    } else {
        show_help();
        return 1;
    }
}