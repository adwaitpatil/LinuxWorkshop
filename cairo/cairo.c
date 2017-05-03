#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <cairo/cairo.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DELAY_IN_SECS    5
#define SCALE_WIDTH      256.0
#define SCALE_HEIGHT     256.0

typedef struct _cairo_linuxfb_device {
    int fb_fd;
    unsigned char *fb_data;
    long fb_screensize;
    struct fb_var_screeninfo fb_vinfo;
    struct fb_fix_screeninfo fb_finfo;
} cairo_linuxfb_device_t;

/* Destroy a cairo surface */
void cairo_linuxfb_surface_destroy(void *device)
{
    cairo_linuxfb_device_t *dev = (cairo_linuxfb_device_t *)device;

    if (dev == NULL)
        return;

    munmap(dev->fb_data, dev->fb_screensize);
    close(dev->fb_fd);
    free(dev);
}

/* Create a cairo surface using the specified framebuffer */
cairo_surface_t *cairo_linuxfb_surface_create(const char *fb_name)
{
    cairo_linuxfb_device_t *device;
    cairo_surface_t *surface;

    /* Use fb0 if no fram buffer is specified */
    if (fb_name == NULL) {
        fb_name = "/dev/fb0";
    }

    device = malloc(sizeof(*device));
    if (!device) {
        perror("Error: cannot allocate memory\n");
        exit(1);
    }

    // Open the file for reading and writing
    device->fb_fd = open(fb_name, O_RDWR);
    if (device->fb_fd == -1) {
        perror("Error: cannot open framebuffer device");
        goto handle_allocate_error;
    }

    // Get variable screen information
    if (ioctl(device->fb_fd, FBIOGET_VSCREENINFO, &device->fb_vinfo) == -1) {
        perror("Error: reading variable information");
        goto handle_ioctl_error;
    }

    // Figure out the size of the screen in bytes
    device->fb_screensize = device->fb_vinfo.xres * device->fb_vinfo.yres
                            * device->fb_vinfo.bits_per_pixel / 8;

    // Map the device to memory
    device->fb_data = (unsigned char *)mmap(0, device->fb_screensize,
                                   PROT_READ | PROT_WRITE, MAP_SHARED,
                                   device->fb_fd, 0);
    if ((int)device->fb_data == -1) {
        perror("Error: failed to map framebuffer device to memory");
        goto handle_ioctl_error;
    }

    // Get fixed screen information
    if (ioctl(device->fb_fd, FBIOGET_FSCREENINFO, &device->fb_finfo) == -1) {
        perror("Error reading fixed information");
        goto handle_ioctl_error;
    }

    /* Create the cairo surface which will be used to draw to */
    surface = cairo_image_surface_create_for_data(device->fb_data,
                  CAIRO_FORMAT_RGB16_565,
                  device->fb_vinfo.xres,
                  device->fb_vinfo.yres,
                  cairo_format_stride_for_width(CAIRO_FORMAT_RGB16_565,
                                                device->fb_vinfo.xres));
    cairo_surface_set_user_data(surface, NULL, device,
                                &cairo_linuxfb_surface_destroy);

    return surface;

handle_ioctl_error:
    close(device->fb_fd);
handle_allocate_error:
    free(device);
    exit(1);
}

int main(int argc, char *argv[]) {
    int image_width;
    int image_height;
    char frame_buffer_number;
    char fb_node[16] = {0};
    cairo_surface_t *surface;
    cairo_surface_t *image;
    cairo_t *cr;

    if (argc != 2) {
        printf("Usage: ./cairo /path/to/png/image\n");
        exit(1);
    }

    if (strstr(argv[1], ".png") == NULL) {
        printf("Only png images are supported with this example\n");
        exit(1);
    }

    printf("Enter frame buffer number:\t");
    scanf("%c", &frame_buffer_number);
    sprintf(fb_node, "/dev/fb");
    fb_node[strlen(fb_node)] = frame_buffer_number;
    fb_node[strlen(fb_node)] = '\0';
    printf("Frame buffer node is: %s\n", fb_node);

    surface = cairo_linuxfb_surface_create(fb_node);
    cr = cairo_create(surface);

    /*
     * We clear the cairo surface here before drawing
     * This is required in case something was drawn on this surface
     * previously, the previous contents would not be cleared without this.
     */
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 32.0);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 100, 300);
    cairo_show_text(cr, "Toradex Cairo Example!");

    /* Wait for the result of drawing operation to persist for the user to see */
    sleep(DELAY_IN_SECS);
    /* Clear the surface and prepare for a new drawing operation */
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    image = cairo_image_surface_create_from_png(argv[1]);
    image_width = cairo_image_surface_get_width(image);
    image_height = cairo_image_surface_get_height(image);

    /* Scale the image arbitrarily */
    cairo_scale(cr, SCALE_WIDTH/image_width, SCALE_HEIGHT/image_height);

    cairo_set_source_surface(cr, image, 350, 200);
    cairo_paint(cr);
    /* Wait for the result of the drawing operation to persist for the user to see */
    sleep(DELAY_IN_SECS);

    /* Destroy and release all cairo related contexts */
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
