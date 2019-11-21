/*
 * hdaps-gl.c - GL-based laptop model that rotates in real-time via hdaps
 *
 * Copyright (C) 2019, the AUTHORS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation and
 * distributed as the COPYING file along with this program.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define UPDATE_THRESHOLD	4
#define SLEEP_INTERVAL		50000	/* microseconds */
#define SYSFS_POSITION_FILE     "/sys/devices/platform/hdaps/position"
#define BUF_LEN                 32
#define WIDTH			640
#define HEIGHT			480

static int val_x;
static int val_y;
static int rest_x;
static int rest_y;

static int fullscreen = 1;
static int window;

/*
 * read_position - read the (x,y) position pair from hdaps.
 *
 * We open and close the file on every invocation, which is lame but due to
 * several features of sysfs files:
 *
 *	(a) Sysfs files are seekable.
 *	(b) Seeking to zero and then rereading does not seem to work.
 *
 * If I were king--and I will be one day--I would of made sysfs files
 * nonseekable and only able to return full-size reads.
 */
static int read_position (int *x, int *y)
{
	char buf[BUF_LEN];
	int fd, ret;

	fd = open (SYSFS_POSITION_FILE, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT) {
			fprintf(stderr,
				"File " SYSFS_POSITION_FILE " not found. "
				"Is the HDAPS kernel module loaded?\n");
		}
		else {
			perror ("open (" SYSFS_POSITION_FILE ")");
		}
		return fd;
	}

	ret = read (fd, buf, BUF_LEN);
	if (ret < 0) {
		perror ("read");
		goto out;
	} else if (ret == 0) {
		fprintf (stderr, "error: unexpectedly read zero!\n");
		ret = 1;
		goto out;
	}
	ret = 0;

	if (sscanf (buf, "(%d,%d)\n", x, y) != 2)
		ret = 1;

out:
	if (close (fd))
		perror ("close");

	return ret;
}

static void resize_scene (int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport (0, 0, width, height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode (GL_MODELVIEW);
}

static void draw_scene (void)
{
	glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glPushMatrix ();

	glRotated (val_x / 2.0, 0.0f, 0.0f, -1.0f);
	glRotated (val_y / 2.0, 1.0f, 0.0f, 0.0f);

	glBegin (GL_QUADS);	// start drawing the laptop.

	// top of body
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 1.6f);
	glVertex3d (-1.0f, 0.0f, 1.6f);
	glVertex3d (-1.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 0.0f);

	// trackpoint
	glColor3d (1.0f, 0.0f, 0.0f);
	glVertex3d (0.01f, 0.01f, 0.78f);
	glVertex3d (0.01f, 0.01f, 0.82f);
	glVertex3d (-0.01f, 0.01f, 0.82f);
	glVertex3d (-0.01f, 0.01f, 0.78f);

	// bottom of body
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, -0.1f, -0.1f);
	glVertex3d (-1.0f, -0.1f, -0.1f);
	glVertex3d (-1.0f, -0.1f, 1.6f);
	glVertex3d (1.0f, -0.1f, 1.6f);

	// front of body
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 1.6f);
	glVertex3d (-1.0f, 0.0f, 1.6f);
	glVertex3d (-1.0f, -0.1f, 1.6f);
	glVertex3d (1.0f, -0.1f, 1.6f);

	// left of body
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (-1.0f, 0.0f, 1.6f);
	glVertex3d (-1.0f, 0.0f, 0.0f);
	glVertex3d (-1.0f, -0.1f, 0.0f);
	glVertex3d (-1.0f, -0.1f, 1.6f);

	// Right of body
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 1.6f);
	glVertex3d (1.0f, -0.1f, 1.6f);
	glVertex3d (1.0f, -0.1f, 0.0f);

	// top of screen
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 1.6f, -0.1f);
	glVertex3d (-1.0f, 1.6f, -0.1f);
	glVertex3d (-1.0f, 1.6f, 0.0f);
	glVertex3d (1.0f, 1.6f, 0.0f);

	// front of screen
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 1.6f, 0.0f);
	glVertex3d (-1.0f, 1.6f, 0.0f);
	glVertex3d (-1.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 0.0f, 0.0f);

	// screen
	glColor3d (0.0f, 0.0f, 1.0f);
	glVertex3d (0.9f, 1.5f, 0.01f);
	glVertex3d (-0.9f, 1.5f, 0.01f);
	glVertex3d (-0.9f, 0.1f, 0.01f);
	glVertex3d (0.9f, 0.1f, 0.01f);

	// back of screen
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, -0.1f, -0.1f);
	glVertex3d (-1.0f, -0.1f, -0.1f);
	glVertex3d (-1.0f, 1.6f, -0.1f);
	glVertex3d (1.0f, 1.6f, -0.1f);

	// left of screen
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (-1.0f, 1.6f, 0.0f);
	glVertex3d (-1.0f, 1.6f, -0.1f);
	glVertex3d (-1.0f, -0.1f, -0.1f);
	glVertex3d (-1.0f, -0.1f, 0.0f);

	// Right of screen
	glColor3d (0.0f, 0.0f, 0.0f);
	glVertex3d (1.0f, 1.6f, -0.1f);
	glVertex3d (1.0f, 1.6f, 0.0f);
	glVertex3d (1.0f, -0.1f, 0.0f);
	glVertex3d (1.0f, -0.1f, -0.1f);

	glEnd ();
	glPopMatrix ();
	glutSwapBuffers ();
}

static void update_scene (void)
{
	int ret, x, y, do_update = 0;

	ret = read_position (&x, &y);
	if (ret)
		exit(EXIT_FAILURE);

	x -= rest_x;
	y -= rest_y;

	/* only update if we surpass our threshold, to minimize jitter ... */
	if (abs (x - val_x) > UPDATE_THRESHOLD) {
		val_x = x;
		do_update = 1;
	}
	if (abs (y - val_y) > UPDATE_THRESHOLD) {
		val_y = y;
		do_update = 1;
	}

	/* ... or, if we are within our threshold of zero, reset to zero */
	if (abs (x) < UPDATE_THRESHOLD) {
		val_x = 0;
		do_update = 1;
	}
	if (abs (y) < UPDATE_THRESHOLD) {
		val_y = 0;
		do_update = 1;
	}

	if (do_update)
		draw_scene ();

	usleep (SLEEP_INTERVAL);
}


static void handle_keyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'f': // exit from full screen
			if (fullscreen == 1) {
				glutPositionWindow (0,0);
				glutReshapeWindow (WIDTH, HEIGHT);
				fullscreen = 0;
			} else {
				glutFullScreen ();
				fullscreen = 1;
			}
			break;
		case 'q': // exit
			glutDestroyWindow (window);
			exit (0);
		case 27: // ESC key - exit
			glutDestroyWindow (window);
			exit (0);
			break;
		}
	glutPostRedisplay();
}

int main (int argc, char *argv[])
{
	int ret;

	ret = read_position (&rest_x, &rest_y);
	if (ret)
		return 1;

	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (WIDTH, HEIGHT);
	glutInitWindowPosition (0, 0);
	window = glutCreateWindow ("IBM Accelerometer Demo");

	glutFullScreen();
	glutKeyboardFunc(&handle_keyboard);

	glutDisplayFunc (&update_scene);
	glutIdleFunc (&update_scene);
	glutReshapeFunc (&resize_scene);

	glClearColor (0.5f, 0.5f, 0.5f, 0.0f);
	glClearDepth (1.0);	// Enables Clearing Of The Depth Buffer
	glDepthFunc (GL_LESS);	// The Type Of Depth Test To Do
	glEnable (GL_DEPTH_TEST);	// Enables Depth Testing
	glShadeModel (GL_SMOOTH);	// Enables Smooth Color Shading

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslated (0.0, -0.5, -4.0);

	draw_scene ();

	/* Start Event Processing Engine */
	glutMainLoop ();

	return 0;
}
