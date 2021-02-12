#ifndef LOG_H
#define LOG_H

#define LOG_OK           0
#define LOG_ERR          1

#define LOG_INFO         (1<<0)
#define LOG_DEBUG        (1<<1)
#define LOG_VERBOSE      (1<<2)

#define LOG_INITDONE    911
#define LOG_MAX_LINE   1024
#define LOG_LINE_WIDTH   80  
#define LOG_MAX_ROW      40  

#define log_mark(M) log_debug("%s [%d]: %s",__FILE__,__LINE__,M);

int log_box_off();
int log_box_on();
int log_close();
int log_debug(const char *, ...);
int log_dump(void *,size_t);
int log_err(const char *, ...);
int log_div();
int log_info(const char *, ...);
int log_init(char *, unsigned char);
int log_set_f(unsigned char);
int log_has_f(unsigned char);
int log_verbose(const char *, ...);
int log_warn(const char *, ...);
void log_fatal(const char *);

#endif /* LOG_H */
