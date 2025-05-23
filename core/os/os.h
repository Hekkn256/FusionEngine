/*************************************************************************/
/*  os.h                                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef OS_H
#define OS_H

#include "types/ustring.h"
#include "list.h"
#include "vector.h"
#include "os/main_loop.h"
#include "scene/resources/texture.h"
#include <stdarg.h>
/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

class OS {

	static OS* singleton;
	String _execpath;
	String _custom_level;
	List<String> _cmdline;
	int ips;
	bool low_processor_usage_mode;
	String _local_clipboard;
	uint64_t frames_drawn;
	uint32_t _frame_delay;
	bool _no_window;
	int _exit_code;
	int _orientation;
	bool camera_enabled = false;

	float _fps;
	int _target_fps;
	float _time_scale;

	char *last_error;

public:
	enum RenderThreadMode {

		RENDER_THREAD_UNSAFE,
		RENDER_THREAD_SAFE,
		RENDER_SEPARATE_THREAD
	};
	struct VideoMode {
	
		int width,height;
		bool fullscreen;
		bool resizable;
		float get_aspect() const { return (float)width/(float)height; }
		VideoMode(int p_width=640,int p_height=480,bool p_fullscreen=false, bool p_resizable = true) { width=p_width; height=p_height; fullscreen=p_fullscreen; resizable = p_resizable; }
	};
protected:
friend class Main;

	bool _verbose_stdout;
	
	RenderThreadMode _render_thread_mode;

	// functions used by main to initialize/deintialize the OS
	virtual int get_video_driver_count() const=0;
	virtual const char * get_video_driver_name(int p_driver) const=0;
	
	virtual VideoMode get_default_video_mode() const=0;
	
	virtual int get_audio_driver_count() const=0;
	virtual const char * get_audio_driver_name(int p_driver) const=0;
	
	virtual void initialize_core()=0;
	virtual void initialize(const VideoMode& p_desired,int p_video_driver,int p_audio_driver)=0;
	
	virtual void set_main_loop( MainLoop * p_main_loop )=0;    
	virtual void delete_main_loop()=0;
	
	virtual void finalize()=0;
	virtual void finalize_core()=0;

	virtual void set_cmdline(const char* p_execpath, const List<String>& p_args);

	void _ensure_data_dir();
	
public:
	Ref<ImageTexture> p_camera_image;
	typedef int64_t ProcessID;

	static OS* get_singleton();

	enum ErrorType {
		ERR_ERROR,
		ERR_WARNING,
		ERR_SCRIPT
	};

	virtual void print_error(const char* p_function,const char* p_file,int p_line,const char *p_code,const char*p_rationale,ErrorType p_type=ERR_ERROR);

	virtual void print(const char *p_format, ... );
	virtual void printerr(const char *p_format, ... );
	virtual void vprint(const char* p_format, va_list p_list, bool p_stderr=false)=0;
	virtual void alert(const String& p_alert,const String& p_title="ALERT!")=0;
	virtual String get_stdin_string(bool p_block = true)=0;

	virtual void set_last_error(const char* p_error);
	virtual const char *get_last_error() const;
	virtual void clear_last_error();




	enum MouseMode {
		MOUSE_MODE_VISIBLE,
		MOUSE_MODE_HIDDEN,
		MOUSE_MODE_CAPTURED
	};

	virtual void set_mouse_mode(MouseMode p_mode);
	virtual MouseMode get_mouse_mode() const;


	virtual void warp_mouse_pos(const Point2& p_to)  {}
	virtual Point2 get_mouse_pos() const=0;
	virtual int get_mouse_button_state() const=0;
	virtual void set_window_title(const String& p_title)=0;

	virtual void set_clipboard(const String& p_text);
	virtual String get_clipboard() const;
	
	virtual void set_video_mode(const VideoMode& p_video_mode,int p_screen=0)=0;
	virtual VideoMode get_video_mode(int p_screen=0) const=0;
	virtual void get_fullscreen_mode_list(List<VideoMode> *p_list,int p_screen=0) const=0;
	
	virtual void set_iterations_per_second(int p_ips);
	virtual int get_iterations_per_second() const;

	virtual void set_target_fps(int p_fps);
	virtual float get_target_fps() const;

	virtual float get_frames_per_second() const { return _fps; };


	virtual void set_low_processor_usage_mode(bool p_enabled);
	virtual bool is_in_low_processor_usage_mode() const;

	virtual String get_executable_path() const;
	virtual Error execute(const String& p_path, const List<String>& p_arguments,bool p_blocking,ProcessID *r_child_id=NULL,String* r_pipe=NULL,int *r_exitcode=NULL)=0;
	virtual Error kill(const ProcessID& p_pid)=0;
	virtual int get_process_ID() const;

	virtual Error shell_open(String p_uri);
	virtual Error set_cwd(const String& p_cwd);

	virtual bool has_environment(const String& p_var) const=0;
	virtual String get_environment(const String& p_var) const=0;

	virtual String get_name()=0;
	virtual List<String> get_cmdline_args() const { return _cmdline; }
	 virtual String get_model_name() const;

	virtual MainLoop *get_main_loop() const=0;

	String get_custom_level() const { return _custom_level; }

	virtual void yield();

	enum Weekday {
		DAY_SUNDAY,
		DAY_MONDAY,
		DAY_TUESDAY,
		DAY_WEDNESDAY,
		DAY_THURSDAY,
		DAY_FRIDAY,
		DAY_SATURDAY
	};
	
	enum Month {
		MONTH_JANUARY,
		MONTH_FEBRUARY,
		MONTH_MARCH,
		MONTH_APRIL,
		MONTH_MAY,
		MONTH_JUNE,
		MONTH_JULY,
		MONTH_AUGUST,
		MONTH_SEPTEMBER,
		MONTH_OCTOBER,
		MONTH_NOVEMBER,
		MONTH_DECEMBER
	};

	struct Date {
		
		int year;
		Month month;
		int day;
		Weekday weekday;
		bool dst;
	};

	struct Time {
	
		int hour;
		int min;
		int sec;
	};
	
	virtual Date get_date() const=0;
	virtual Time get_time() const=0;
	virtual uint64_t get_unix_time() const;

	virtual void delay_usec(uint32_t p_usec) const=0; 
	virtual uint64_t get_ticks_usec() const=0;
	uint32_t get_ticks_msec() const;

	void set_frame_delay(uint32_t p_msec);
	uint32_t get_frame_delay() const;

	virtual bool can_draw() const = 0;

	uint64_t get_frames_drawn();

	bool is_stdout_verbose() const;

	enum CursorShape {
		CURSOR_ARROW,
		CURSOR_IBEAM,
		CURSOR_POINTING_HAND,
		CURSOR_CROSS,
		CURSOR_WAIT,
		CURSOR_BUSY,
		CURSOR_DRAG,
		CURSOR_CAN_DROP,
		CURSOR_FORBIDDEN,
		CURSOR_VSIZE,
		CURSOR_HSIZE,
		CURSOR_BDIAGSIZE,
		CURSOR_FDIAGSIZE,
		CURSOR_MOVE,
		CURSOR_VSPLIT,
		CURSOR_HSPLIT,
		CURSOR_HELP,
		CURSOR_MAX
	};


	virtual bool has_virtual_keyboard() const;
	virtual void show_virtual_keyboard(const String& p_existing_text,const Rect2& p_screen_rect=Rect2());
	virtual void hide_virtual_keyboard();

	virtual void set_cursor_shape(CursorShape p_shape)=0;

	virtual bool get_swap_ok_cancel() { return false; }
	virtual void dump_memory_to_file(const char* p_file);
	virtual void dump_resources_to_file(const char* p_file);
	virtual void print_resources_in_use(bool p_short=false);
	virtual void print_all_resources(String p_to_file="");

	virtual int get_static_memory_usage() const;
	virtual int get_static_memory_peak_usage() const;
	virtual int get_dynamic_memory_usage() const;
	virtual int get_free_static_memory() const;

	RenderThreadMode get_render_thread_mode() const { return _render_thread_mode; }

	virtual String get_locale() const;

	virtual String get_data_dir() const;
	virtual String get_resource_dir() const;

	enum SystemDir {
		SYSTEM_DIR_DESKTOP,
		SYSTEM_DIR_DCIM,
		SYSTEM_DIR_DOCUMENTS,
		SYSTEM_DIR_DOWNLOADS,
		SYSTEM_DIR_MOVIES,
		SYSTEM_DIR_MUSIC,
		SYSTEM_DIR_PICTURES,
		SYSTEM_DIR_RINGTONES,
	};

	virtual String get_system_dir(SystemDir p_dir) const;


	virtual void set_no_window_mode(bool p_enable);
	virtual bool is_no_window_mode_enabled() const;

	virtual bool has_touchscreen_ui_hint() const;

	enum ScreenOrientation {

		SCREEN_LANDSCAPE,
		SCREEN_PORTRAIT,
		SCREEN_REVERSE_LANDSCAPE,
		SCREEN_REVERSE_PORTRAIT,
		SCREEN_SENSOR_LANDSCAPE,
		SCREEN_SENSOR_PORTRAIT,
		SCREEN_SENSOR,
	};

	virtual void set_screen_orientation(ScreenOrientation p_orientation);
	ScreenOrientation get_screen_orientation() const;

	virtual void move_window_to_foreground() {};

	virtual void debug_break();

	virtual void release_rendering_thread();
	virtual void make_rendering_thread();
	virtual void swap_buffers();
	virtual void set_camera_enabled(bool p_enable);
	virtual bool get_camera_enabled() {return camera_enabled;};
	virtual void set_camera_image(Ref<ImageTexture> data);
	virtual Ref<ImageTexture> get_camera_image() {return p_camera_image;};

	virtual void set_icon(const Image& p_icon);

	virtual int get_exit_code() const;
	virtual void set_exit_code(int p_code);

	virtual int get_processor_count() const;

	virtual String get_unique_ID() const;

	virtual Error native_video_play(String p_path, float p_volume, String p_audio_track, String p_subtitle_track);
	virtual bool native_video_is_playing() const;
	virtual void native_video_pause();
	virtual void native_video_stop();

	virtual bool can_use_threads() const;

	virtual Error dialog_show(String p_title, String p_description, Vector<String> p_buttons, Object* p_obj, String p_callback);
	virtual Error dialog_input_text(String p_title, String p_description, String p_partial, Object* p_obj, String p_callback);

	enum Charset {
		OS_CHARSET_ANSI,
		OS_CHARSET_UTF8,
		OS_CHARSET_UNICODE,
	};
	virtual Charset get_charset() const { return OS_CHARSET_UTF8; }

	void set_time_scale(float p_scale);
	float get_time_scale() const;

	OS();	
	virtual ~OS();

};

#endif

