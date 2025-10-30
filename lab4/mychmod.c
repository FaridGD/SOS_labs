#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int isoctal(char* s) {
    if(!s || !*s) return 0;
    for(char* p = s; *p; p++) {
        if(*p < '0' || *p > '7') return 0;
    }
    return 1;
}

mode_t getoctal(char* s) {
    char* end;
    long val = strtol(s, &end, 8);
    if(end == s) return (mode_t)-1;
    return (mode_t)val;
}

void showuse(char* prog) {
    fprintf(stderr, "Usage: %s MODE FILE\n", prog);
    exit(2);
}

int dooctal(char* mode, char* path) {
    mode_t m = getoctal(mode);
    if(m == (mode_t)-1){
        fprintf(stderr, "Invalid octal mode: %s\n", mode);
        return -1;
    }
    if(chmod(path, m) != 0){
        perror("chmod");
        return -1;
    }
    return 0;
}

int dosymbol(char* mode, char* path) {
    struct stat st;
    if(stat(path, &st) != 0){
        perror("stat");
        return -1;
    }
    
    mode_t newmode = st.st_mode;
    char* p = mode;
    
    while(*p) {
        int u = 0, g = 0, o = 0, any = 0;
        char* start = p;
        
        while(*p && (*p=='u' || *p=='g' || *p=='o' || *p=='a')) {
            if(*p == 'u') u = 1;
            else if(*p == 'g') g = 1;
            else if(*p == 'o') o = 1;
            else if(*p == 'a') u = g = o = 1;
            p++;
            any = 1;
        }
        
        if(!any) u = g = o = 1;
        if(!*p) return -1;
        
        char op = *p++;
        if(op != '+' && op != '-' && op != '=') return -1;
        
        int r = 0, w = 0, x = 0, s = 0, t = 0, got = 0;
        
        while(*p && *p != ',') {
            char c = *p++;
            if(c == 'r') { r = 1; got = 1; }
            else if(c == 'w') { w = 1; got = 1; }
            else if(c == 'x') { x = 1; got = 1; }
            else if(c == 's') { s = 1; got = 1; }
            else if(c == 't') { t = 1; got = 1; }
            else return -1;
        }
        
        if(!got) return -1;
        
        mode_t perm = 0;
        mode_t spec = 0;
        
        if(r) {
            if(u) perm |= S_IRUSR;
            if(g) perm |= S_IRGRP;
            if(o) perm |= S_IROTH;
        }
        if(w) {
            if(u) perm |= S_IWUSR;
            if(g) perm |= S_IWGRP;
            if(o) perm |= S_IWOTH;
        }
        if(x) {
            if(u) perm |= S_IXUSR;
            if(g) perm |= S_IXGRP;
            if(o) perm |= S_IXOTH;
        }
        if(s) {
            if(u) spec |= S_ISUID;
            if(g) spec |= S_ISGID;
        }
        if(t) spec |= S_ISVTX;
        
        if(op == '+') {
            newmode |= (perm | spec);
        } else if(op == '-') {
            newmode &= ~(perm | spec);
        } else if(op == '=') {
            if(u) {
                newmode &= ~(S_IRUSR|S_IWUSR|S_IXUSR);
                newmode |= (r?S_IRUSR:0) | (w?S_IWUSR:0) | (x?S_IXUSR:0);
                if(s) newmode |= S_ISUID;
                else newmode &= ~S_ISUID;
            }
            if(g) {
                newmode &= ~(S_IRGRP|S_IWGRP|S_IXGRP);
                newmode |= (r?S_IRGRP:0) | (w?S_IWGRP:0) | (x?S_IXGRP:0);
                if(s) newmode |= S_ISGID;
                else newmode &= ~S_ISGID;
            }
            if(o) {
                newmode &= ~(S_IROTH|S_IWOTH|S_IXOTH);
                newmode |= (r?S_IROTH:0) | (w?S_IWOTH:0) | (x?S_IXOTH:0);
            }
            if(t) newmode |= S_ISVTX;
            else newmode &= ~S_ISVTX;
        }
        
        if(*p == ',') p++;
    }
    
    mode_t final = newmode & (S_ISUID|S_ISGID|S_ISVTX|0777);
    if(chmod(path, final) != 0){
        perror("chmod");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 3) showuse(argv[0]);
    
    char* mode = argv[1];
    char* path = argv[2];
    
    if(isoctal(mode)) {
        if(dooctal(mode, path) != 0) return 1;
    } else {
        if(dosymbol(mode, path) != 0) return 1;
    }
    
    return 0;
}