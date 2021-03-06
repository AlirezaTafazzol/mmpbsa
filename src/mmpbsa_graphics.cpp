#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_GRAPHICS

// This file largely barrows from uc2_graphics.cpp from BOINC, with changes
// made to incorporate molecule representations. The following
// is the terms of copying that code, which was taken from the beginning of
// the file:
//
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Example graphics application, paired with uc2.C
// This demonstrates:
// - using shared memory to communicate with the worker app
// - reading XML preferences by which users can customize graphics
//   (in this case, select colors)
// - handle mouse input (in this case, to zoom and rotate)
// - draw text and 3D objects using OpenGL
#ifdef _WIN32
#include "boinc/boinc_win.h"
#else
#include <math.h>
#endif

#include "gl.h"
#include "glu.h"

//BOINC stuff
#include "boinc/parse.h"
#include "boinc/util.h"
//#include "boinc/gutil.h"
#include "boinc/boinc_gl.h"
#include "boinc/app_ipc.h"
#include "boinc/boinc_api.h"
#include "boinc/graphics2.h"
#include "boinc/txf_util.h"
#include "boinc/diagnostics.h"
#ifdef __APPLE__
#include "boinc/mac/app_icon.h"
#endif

#include "mmpbsa_graphics.h"
#include "mmpbsa_gutil.h"

//mmpbsa::SanderParm * parminfo;
size_t natoms;
float*  crds;
float white[4] = {1., 1., 1., 1.};
float black[4] = {0,0,0,1};
float blue[4] = {0,0,1,1};
TEXTURE_DESC logo;
int width, height;      // window dimensions
APP_INIT_DATA uc_aid;
bool mouse_down = false;
int mouse_x, mouse_y;
double pitch_angle, roll_angle, viewpoint_distance=10;
float color[4] = {.7, .2, .5, 1};
float red[4] = {1,0,0,1};
    // the color of the 3D object.
    // Can be changed using preferences
MMPBSA_SHMEM* shmem = NULL;

// set up lighting model
//
static void init_lights() {
   GLfloat ambient[] = {1., 1., 1., 1.0};
   GLfloat position[] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
}

static void draw_logo() {
    if (logo.present) {
        float pos[3] = {.2, .3, 0};
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
    }
}

int* formatTime(const int& seconds)
{
    int * returnMe = new int[3];
    returnMe[2] = seconds % 60;
    returnMe[1] = (seconds/60) % 60;
    returnMe[0] = seconds/3600;
    return returnMe;
}

static void draw_text() {
    static float x=0, y=0;
    static float dx=0.0003, dy=0.0007;
    char buf[256];
    x += dx;
    y += dy;
    if (x < 0 || x > .5) dx *= -1;
    if (y < 0 || y > .5) dy *= -1;
    double fd = 0, cpu=0, dt;
    if (shmem) {
        fd = shmem->fraction_done;
        cpu = shmem->cpu_time;
    }

    if(strlen(uc_aid.user_name))
    {
        sprintf(buf, "User: %s", uc_aid.user_name);
        txf_render_string(.1, x, y, 0, 500, white, 0, buf);
    }
//    sprintf(buf, "Team: %s", uc_aid.team_name);
//    txf_render_string(.1, x, y+.1, 0, 500, white, 0, buf);
    sprintf(buf, "Progress: %5.2f%%", 100*fd);
    txf_render_string(.1, x, y+.2, 0, 500, white, 0, buf);
    int* formattedTime = formatTime(int(cpu));
    sprintf(buf, "CPU time: %d hrs %d mins %d secs", formattedTime[0],formattedTime[1], formattedTime[2]);
    delete[] formattedTime;
    txf_render_string(.1, x, y+.3, 0, 500, white, 0, buf);
    if (shmem) {
        dt = dtime() - shmem->update_time;
        if (dt > 10) {
            boinc_close_window_and_quit("shmem not updated");
        } else if (dt > 5) {
            txf_render_string(.1, 0, 0, 0, 500, white, 0, "App not running - exiting in 5 seconds");
        } else if (shmem->status.suspended) {
            txf_render_string(.1, 0, 0, 0, 500, white, 0, "App suspended");
        }
    } else {
        txf_render_string(.1, 0, 0, 0, 500, white, 0, "No shared mem");
    }
}

static void draw_atom(float* index)
{
  float pos[3];
  pos[0] = index[0];
  pos[1] = index[1];
  pos[2] = index[2];

  float* itsColor;
  float itsRadius = 3;
  float radiusScale = 2;
  switch(int(index[3]))
    {
    case 0:
      itsColor = &red[0];
      break;
    case 2:
      itsColor = &black[0];
      break;
    case 3:
      itsColor = &blue[0];
      break;
    case 1: default:
      itsColor = &white[0];
      break;
    }
  mode_shaded(itsColor);
  drawSphere(pos, itsRadius);
}

static void draw_3d_stuff(float* crds,size_t natoms) {
    static float x=0, y=0, z=10;
    static float dx=0.3, dy=0.2, dz=0.5;
    x += dx;
    y += dy;
    z += dz;
    if (x < -15 || x > 15) dx *= -1;
    if (y < -15 || y > 15) dy *= -1;
    if (z < 0 || z > 40) dz *= -1;
    float pos[4];
    for(size_t i = 0;i<natoms*4;i+=4)
      {
	pos[0] = crds[i] +  x;
	pos[1] = crds[i+1] + y;
	pos[2] = crds[i+2] + z;
	pos[3] = crds[i+3];
	draw_atom(pos);
      }

}

void set_viewpoint(double dist) {
    double x, y, z;
    x = 0;
    y = 3.0*dist;
    z = 11.0*dist;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        x, y, z,        // eye position
        0,-.8,0,        // where we're looking
        0.0, 1.0, 0.    // up is in positive Y direction
    );
    glRotated(pitch_angle, 1., 0., 0);
    glRotated(roll_angle, 0., 1., 0);
}

static void init_camera(double dist) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        45.0,       // field of view in degree
        1.0,        // aspect ratio
        1.0,        // Z near clip
        1000.0      // Z far
    );
    set_viewpoint(dist);
}

void app_graphics_render(int xs, int ys, double time_of_day) {
    // boinc_graphics_get_shmem() must be called after
    // boinc_parse_init_data_file()
    // Put this in the main loop to allow retries if the
    // worker application has not yet created shared memory
    //
    if (shmem == NULL) {
        shmem = (MMPBSA_SHMEM*)boinc_graphics_get_shmem("mmpbsa");
    }
    if (shmem) {
        shmem->countdown = 5;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw logo first - it's in background
    //
    mode_unshaded();
    mode_ortho();
    draw_logo();
    ortho_done();

    // draw 3D objects
    //
    init_camera(viewpoint_distance);
    scale_screen(width, height);
    mode_shaded(color);
    draw_3d_stuff(crds,natoms);

    // draw text on top
    //
    mode_unshaded();
    mode_ortho();
    draw_text();
    ortho_done();
}

void app_graphics_resize(int w, int h){
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

// mouse drag w/ left button rotates 3D objects;
// mouse draw w/ right button zooms 3D objects
//
void boinc_app_mouse_move(int x, int y, int left, int middle, int right) {
    if (left) {
        pitch_angle += (y-mouse_y)*.1;
        roll_angle += (x-mouse_x)*.1;
        mouse_y = y;
        mouse_x = x;
    } else if (right) {
        double d = (y-mouse_y);
        viewpoint_distance *= exp(d/100.);
        mouse_y = y;
        mouse_x = x;
    } else {
        mouse_down = false;
    }
}

void boinc_app_mouse_button(int x, int y, int which, int is_down) {
    if (is_down) {
        mouse_down = true;
        mouse_x = x;
        mouse_y = y;
    } else {
        mouse_down = false;
    }
}

void boinc_app_key_press(int, int){}

void boinc_app_key_release(int, int){}

void app_graphics_init() {
    char path[256];

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    txf_load_fonts(".");

    boinc_resolve_filename("logo.bmp", path, sizeof(path));
    logo.load_image_file(path);

    init_lights();
}

static void parse_project_prefs(char* buf) {
    char cs[256];
    COLOR c;
    double hue;
    double max_frames_sec, max_gfx_cpu_pct;
    if (!buf) return;
    if (parse_str(buf, "<color_scheme>", cs, 256)) {
        if (!strcmp(cs, "Tahiti Sunset")) {
            hue = .9;
        } else if (!strcmp(cs, "Desert Sands")) {
            hue = .1;
        } else {
            hue = .5;
        }
        HLStoRGB(hue, .5, .5, c);
        color[0] = c.r;
        color[1] = c.g;
        color[2] = c.b;
        color[3] = 1;
    }
    if (parse_double(buf, "<max_frames_sec>", max_frames_sec)) {
        boinc_max_fps = max_frames_sec;
    }
    if (parse_double(buf, "<max_gfx_cpu_pct>", max_gfx_cpu_pct)) {
        boinc_max_gfx_cpu_frac = max_gfx_cpu_pct/100;
    }
}

#endif//use graphics

int main(int argc, char** argv)
{
#ifndef USE_GRAPHICS
    std::cout << "Graphics application has not been build. Run ./configure --with-graphics to build graphics application. " << std::endl;
    return 0;
#else

    boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);

//#ifdef __APPLE__
//    setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
//#endif

    natoms = 11;
    crds = new float[4*natoms];
    crds[0] = 0;
    crds[1] = 0;
    crds[2] = 0;
    crds[3] = 2;

    crds[4] = 3;
    crds[5] = 3;
    crds[6] = 0;
    crds[7] = 0;

    crds[8] = 2;
    crds[9] = 4;
    crds[10] = 0;
    crds[11] = 1;

    crds[12] = -3;
    crds[13] = 3;
    crds[14] = 0;
    crds[15] = 0;

    crds[16] = 0;
    crds[17] = -3;
    crds[18] = 0;
    crds[19] = 2;

    crds[20] = -3;
    crds[21] = -6;
    crds[22] = 0;
    crds[23] = 2;

    crds[24] = -5;
    crds[25] = -3;
    crds[26] = 0;
    crds[27] = 1;

    crds[28] = -5;
    crds[29] = -8;
    crds[30] = 0;
    crds[31] = 1;

    crds[32] = 3;
    crds[33] = -6;
    crds[34] = 0;
    crds[35] = 3;

    crds[36] = 5;
    crds[37] = -4;
    crds[38] = 0;
    crds[39] = 1;

    crds[40] = 5;
    crds[41] = -8;
    crds[42] = 0;
    crds[43] = 1;/**/


    boinc_parse_init_data_file();
    boinc_get_init_data(uc_aid);
    if (uc_aid.project_preferences) {
        parse_project_prefs(uc_aid.project_preferences);
    }
    boinc_graphics_loop(argc, argv);
    //delete parminfo;
    delete [] crds;
    boinc_finish_diag();


#endif //not USE_GRAPHICS
}



