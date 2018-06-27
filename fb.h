#ifndef fb_h
#define fb_h
#ifdef __cplusplus 
extern "C" {
#endif
void fb_put_pixel(int x, int y, int r, int g, int b);
int fb_init();
void fb_cleanup();
#ifdef __cplusplus
}
#endif
#endif