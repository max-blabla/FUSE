

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <pthread.h>


#define MAX_FILE_LENGTH 1<<15
#define MAX_FILE_NAME_LENGTH 1 << 6
#define MAX_FILE_NUM 1 << 7
#define MAX_USER_NUM 1 << 4

struct File{
    char * data;
    char * file_name;
    int len;
};


struct User{
    char * user_name;
    struct File * sub_file;
    int file_num;
};

struct User * All_User;
int UserNum = 0;


pthread_mutex_t lock ; // avoid conflicts while using rbtree




static void *bot_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
}

static struct User * searchUser(char * UserName){
    for(int i = 0 ; i < UserNum ; ++i)
        if(strcmp(UserName,All_User[i].user_name) == 0) return All_User + i;
    return NULL;
}

static void Parse(const char * Path, char * User, char * File){
    int len = strlen(Path);
    int p = 1;
    for(int i = p ; i < len ; ++i ){
        if(Path[i] == '/') break;
        User[i-1] = Path[i];
        ++p;
    }
    User[p-1] = '\0';
    int q = ++p;
    for(int i = p ; i < len ; ++i ){
        if(Path[i] == '/') break;
        File[i-q] = Path[i];
        ++p;
    }
    File[p-q] = '\0';
}

static struct File * searchFile(struct User * user, char * Filename){
    for(int i = 0 ; i < user->file_num ; ++i)
        if(strcmp((user->sub_file)[i].file_name,Filename) == 0) return user->sub_file+i;
    return NULL;
}

static struct User * createUser(const char * Username){
    for(int i = 0 ; i < UserNum ; ++i)
        if(strcmp(Username,All_User[i].user_name) == 0) return NULL;
    All_User[UserNum].user_name = (char  *) malloc (MAX_FILE_NAME_LENGTH);
    All_User[UserNum].sub_file = (struct File * ) malloc ((MAX_FILE_NUM) * sizeof(struct File));
    All_User[UserNum].file_num = 0;
    strcpy(All_User[UserNum].user_name,Username);
    int index = UserNum;
    ++UserNum;
    return All_User+index;
}
static struct File * createFile(char * UserName, char * FileName){
    struct User * user=  NULL;
    for(int i = 0 ; i < UserNum ; ++i)
        if(strcmp(UserName,All_User[i].user_name) == 0){
         user = All_User + i;
        }
    if(user) {
    for(int i = 0 ; i < user->file_num ; ++i){
        if(strcmp(FileName,(user->sub_file)[i].file_name) == 0)    return NULL;
        }
        (user->sub_file)[user->file_num].file_name = (char *)malloc(MAX_FILE_NAME_LENGTH);
        (user->sub_file)[user->file_num].data =  (char *)malloc(MAX_FILE_LENGTH);
        (user->sub_file)[user->file_num].len =  MAX_FILE_LENGTH;
        strcpy((user->sub_file)[user->file_num].file_name,FileName);
        int index = user -> file_num;
   	++(user -> file_num);
        return (user->sub_file) + index;
    }
    else return NULL;
}
int msg = 0;
static struct File * FindFile(char * UserName, char * FileName){
    struct File * file=  NULL;
    struct User * user=  NULL;
    for(int i = 0 ; i < UserNum ; ++i){
        if(strcmp(UserName,All_User[i].user_name) == 0) {
            user = All_User + i;
            for (int j = 0; j < user->file_num; ++j) {
                if(strcmp(FileName, (user->sub_file)[j].file_name) == 0){
                 	file = (user->sub_file) + j;
                 	break;
                 }
            }
            break;
        }
    }
    return file;
}


static int bot_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{

    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));


    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0 ;
    }

    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName);
    pthread_mutex_lock(&lock) ;
    struct User * user = searchUser(UserName);
    if (!user){ 
    	res = -ENOENT;
    }
    else{
    struct File * file = searchFile(user,FileName);
        if (strlen(FileName) == 0){
            stbuf->st_mode = S_IFDIR | 0755 ;
            stbuf->st_nlink = 2 ;
        }else if(!file){
            res = -ENOENT;
        }
        else{
            stbuf->st_mode = S_IFREG | 0444 ;
            stbuf->st_nlink = 1 ;
            stbuf->st_size = file->len ;
        }
    }

    pthread_mutex_unlock(&lock) ;

    return res ;



}

static int bot_access(const char* path , int mask){

    int res = 0 ;


    return res ;

}


static int bot_mkdir(const char* path, mode_t mode){
    int res = 0 ;
    // create user
    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName);
    pthread_mutex_lock(&lock) ;
    struct User * user = createUser(UserName) ;

    if (!user){
        res = -ENOENT;
    }
    pthread_mutex_unlock(&lock) ;
    return res ;

}

static int bot_mknod(const char *path,mode_t mode, dev_t rdev){
    int res = 0 ;
    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName) ; 
    pthread_mutex_lock(&lock) ;
    struct File* file = createFile(UserName,FileName) ;
    if (!file){
        res = -EEXIST ;
    }

    pthread_mutex_unlock(&lock) ;
    return res ;

}

static int bot_open(const char *path , struct fuse_file_info *fi){

    int res = 0 ;
    return res ;

}

static int bot_release(const char *path, struct fuse_file_info *fi){
    return 0 ;
}

static int bot_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{


/*    if(strcmp(path+1, options.filename) == 0){
        int len = strlen(options.contents);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, options.contents + offset, size);
        } else{
            size = 0;
        }
        return size ;
    }*/

    struct File* file ;
    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName) ; // /usr1 + /usr2
    pthread_mutex_lock(&lock) ;

    file = FindFile(UserName,FileName);

    pthread_mutex_unlock(&lock) ;
    if(!file) return -ENOENT;
    int file_size = file->len ;
    if ( offset > file_size ) return 0 ;
    int avail = file_size - offset ;
    int rsize = size < avail ? size : avail ;
    memcpy(buf,file->data+offset,rsize) ;
    return rsize;

}

static int bot_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    struct File* file ;
    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName) ; // /usr1 + /usr2
    
    pthread_mutex_lock(&lock) ;
    file = FindFile(UserName,FileName);
    pthread_mutex_unlock(&lock) ;
    if(!file) return msg;
    int file_size = file->len ;
    int min_size = offset + size ;
    memcpy(file->data+offset,buf,size) ;
    file->len = file_size > min_size ? file_size : min_size ;

    return size ;

}

static int bot_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{

    //int res = 0 , usr_flag = 0  ;
    //int is_root = 0; 
    //if (strcmp(path, "/") == 0){ // print all the users
    //    is_root = 1 ;
    //  filler(buf, options.filename, NULL, 0, 0);
    //}

    char UserName[MAX_FILE_NAME_LENGTH];
    char FileName[MAX_FILE_NAME_LENGTH];
    Parse(path,UserName,FileName) ; // /usr1 + /usr2
    
    pthread_mutex_lock(&lock) ;
    
    filler(buf,".",NULL,0,0) ;
    filler(buf,"..",NULL,0,0) ;
    if(strlen(UserName)==0){
    	for(int i = 0 ; i < UserNum ; ++i)
 		filler(buf, All_User[i].user_name, NULL, 0, 0);
    }
    else{
    	struct User * user = searchUser(UserName);
    	for(int i =0 ; i < user->file_num ; ++i){
    		filler(buf,(user->sub_file)[i].file_name,NULL,0,0);
    	}
    }


/*    struct rb_node *iter = NULL ;
    for (iter = rb_first(&root);iter;iter=rb_next(iter)){
        struct bot_file* tmp_file = rb_entry(iter,struct bot_file,node) ;
        if (isUser(tmp_file->path)){
            if (usr_flag == 1){
                filler(buf,tmp_file->path+1,NULL,0,0) ;
            }
        }else if (usr_flag == 0 && strcmp(tmp_file->from_user,path)==0){
            filler(buf,tmp_file->to_user+1,NULL,0,0) ;
        }
    }*/


    pthread_mutex_unlock(&lock) ;


    return 0;

}

static struct fuse_operations bot_operation = {
        .init       = bot_init,
        .getattr    = bot_getattr,
        .access     = bot_access,
        .readdir    = bot_readdir,

        .open       = bot_open,
        .read       = bot_read,
        .write      = bot_write,
        .release    = bot_release,

        .mknod      = bot_mknod,

        .mkdir      = bot_mkdir

};

int main(int argc, char *argv[])
{
    All_User = (struct User *) malloc((MAX_USER_NUM) * sizeof(struct  User));
    int ret = fuse_main(argc, argv, &bot_operation, NULL);
    return ret;
}

