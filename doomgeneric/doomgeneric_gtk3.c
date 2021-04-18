#include "doomkeys.h"

#include "doomgeneric.h"
#include "i_system.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>

#include <gtk/gtk.h>


#define KEYQUEUE_SIZE 16
  
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static GtkWidget* mainWindow = NULL;
static GdkPixbuf* pixbuf = NULL;
static bool windowOpen = false;




enum GtkKeys{GTK_RIGHT_ARROW = 65363, 
             GTK_LEFT_ARROW = 65361, 
             GTK_UP_ARROW = 65362, 
             GTK_DOWN_ARROW = 65364,
             GTK_ENTER = 65293,
             GTK_ESCAPE = 65307,
             GTK_CTRL_L = 65507,
             GTK_CTRL_R = 65508,
             GTK_SPACE = 32,
             GTK_SHIFT_L = 65505,
             GTK_SHIFT_R = 65506
             };





static unsigned char convertToDoomKey(unsigned int key)
{
	switch (key)
	{
    case GTK_ENTER:
		key = KEY_ENTER;
		break;
    case GTK_ESCAPE:
		key = KEY_ESCAPE;
		break;
    case GTK_LEFT_ARROW:
		key = KEY_LEFTARROW;
		break;
    case GTK_RIGHT_ARROW:
		key = KEY_RIGHTARROW;
		break;
    case GTK_UP_ARROW:
		key = KEY_UPARROW;
		break;
    case GTK_DOWN_ARROW:
		key = KEY_DOWNARROW;
		break;
    case GTK_CTRL_L:
    case GTK_CTRL_R:
		key = KEY_FIRE;
		break;
    case GTK_SPACE:
		key = KEY_USE;
		break;
    case GTK_SHIFT_L:
    case GTK_SHIFT_R:
		key = KEY_RSHIFT;
		break;
	default:
		key = tolower(key);
		break;
	}

	return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode)
{
	unsigned char key = convertToDoomKey(keyCode);

	unsigned short keyData = (pressed << 8) | key;

	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

GCallback onWindowDestroy(void) {
    windowOpen = false;
    I_Quit();

    return 0;
}

GCallback onKeyPressed(GtkWidget *widget, GdkEventKey *event) {
    addKeyToQueue(1, event->keyval);

    return 0;
}

GCallback onKeyReleased(GtkWidget *widget, GdkEventKey *event) {
    addKeyToQueue(0, event->keyval);

    return 0;
}

GCallback onDraw(GtkWidget *widget, cairo_t* cr) {
    if (pixbuf != NULL) {
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
    }

    return 0;
}

void DG_Init() {
    gtk_init(NULL, NULL);

    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(mainWindow), DOOMGENERIC_RESX, DOOMGENERIC_RESY);
    GtkWidget* drawingArea = gtk_drawing_area_new();

    
    gtk_container_add(GTK_CONTAINER(mainWindow), drawingArea);

    g_signal_connect(GTK_WINDOW(mainWindow), "destroy", (GCallback)onWindowDestroy, NULL);
    g_signal_connect(GTK_WINDOW(mainWindow), "key_press_event", (GCallback)onKeyPressed, NULL);
    g_signal_connect(GTK_WINDOW(mainWindow), "key_release_event", (GCallback)onKeyReleased, NULL);
    g_signal_connect(GTK_DRAWING_AREA(drawingArea), "draw", (GCallback)onDraw, NULL);

    pixbuf = gdk_pixbuf_new_from_data((guchar*)DG_ScreenBuffer,
                                      GDK_COLORSPACE_RGB,
                                      false,
                                      8,
                                      DOOMGENERIC_RESX,
                                      DOOMGENERIC_RESY,
                                      DOOMGENERIC_RESX*3,
                                      NULL, NULL);

    gtk_widget_show_all(mainWindow);
    windowOpen = true;
    gtk_main_iteration_do(FALSE);
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }
}

void DG_DrawFrame() {
  if (windowOpen) {
        if (pixbuf == NULL) {
            g_print("ERROR: Pixbuf is null\n");
        }
        gtk_widget_queue_draw(mainWindow);
        while (gtk_events_pending()) {
            gtk_main_iteration_do(FALSE);
        }
        
    }
}

void DG_SleepMs(uint32_t ms) {
    usleep (ms * 1000);
}

uint32_t DG_GetTicksMs() {
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
	if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
	{
		//key queue is empty

		return 0;
	}
	else
	{
		unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
		s_KeyQueueReadIndex++;
		s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

		*pressed = keyData >> 8;
		*doomKey = keyData & 0xFF;

		return 1;
	}

}

void DG_SetWindowTitle(const char* title) {
    if (mainWindow != NULL) {
        gtk_window_set_title(GTK_WINDOW(mainWindow), title);
    }
}
