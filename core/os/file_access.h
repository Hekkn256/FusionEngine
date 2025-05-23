/*************************************************************************/
/*  file_access.h                                                        */
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
#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include "typedefs.h"
#include "types/ustring.h"
#include "os/memory.h"
#include "math_defs.h"
/**
 * Multi-Platform abstraction for accessing to files.
 */

class FileAccess {

public:
	enum AccessType {
		ACCESS_RESOURCES,
		ACCESS_USERDATA,
		ACCESS_FILESYSTEM,
		ACCESS_MAX
	};

	typedef FileAccess*(*CreateFunc)();
	bool endian_swap;
	bool real_is_double;
protected:

	String fix_path(const String& p_path) const;
	virtual Error _open(const String& p_path, int p_mode_flags)=0; ///< open a file
	virtual uint64_t _get_modified_time(const String& p_file)=0;


private:

	static bool backup_save;

	AccessType _access_type;
	static CreateFunc create_func[ACCESS_MAX]; /** default file access creation function for a platform */
	template<class T>
	static FileAccess* _create_builtin() {

		return memnew( T );
	}

public:

	virtual void _set_access_type(AccessType p_access);

	enum ModeFlags  {

		READ=1,
		WRITE=2,
		READ_WRITE=3,
	};
	
	virtual void close()=0; ///< close a file 
	virtual bool is_open() const=0; ///< true when file is open 

	virtual void seek(size_t p_position)=0; ///< seek to a given position 
	virtual void seek_end(int64_t p_position=0)=0; ///< seek from the end of file 
	virtual size_t get_pos() const=0; ///< get position in the file 
	virtual size_t get_len() const=0; ///< get size of the file 

	virtual bool eof_reached() const=0; ///< reading passed EOF 

	virtual uint8_t get_8() const=0; ///< get a byte 
	virtual uint16_t get_16() const; ///< get 16 bits uint  
	virtual uint32_t get_32() const; ///< get 32 bits uint 
	virtual uint64_t get_64() const; ///< get 64 bits uint 

	virtual float get_float() const;
	virtual double get_double() const;
	virtual real_t get_real() const;

	virtual int get_buffer(uint8_t *p_dst,int p_length) const; ///< get an array of bytes
	virtual String get_line() const;
	virtual Vector<String> get_csv_line() const;
	
	/**< use this for files WRITTEN in _big_ endian machines (ie, amiga/mac)
	 * It's not about the current CPU type but file formats.
	 * this flags get reset to false (little endian) on each open
	 */
	 
	virtual void set_endian_swap(bool p_swap) { endian_swap=p_swap; }
	inline bool get_endian_swap() const { return endian_swap; }

	virtual Error get_error() const=0; ///< get last error 

	virtual void store_8(uint8_t p_dest)=0; ///< store a byte 
	virtual void store_16(uint16_t p_dest); ///< store 16 bits uint 
	virtual void store_32(uint32_t p_dest); ///< store 32 bits uint 
	virtual void store_64(uint64_t p_dest); ///< store 64 bits uint 

	virtual void store_float(float p_dest);
	virtual void store_double(double p_dest);
	virtual void store_real(real_t p_real);

	virtual void store_string(const String& p_string);
	virtual void store_line(const String& p_string);

	virtual void store_pascal_string(const String& p_string);
	virtual String get_pascal_string();

	virtual void store_buffer(const uint8_t *p_src,int p_length); ///< store an array of bytes 
	
	virtual bool file_exists(const String& p_name)=0; ///< return true if a file exists 

	virtual Error reopen(const String& p_path, int p_mode_flags); ///< does not change the AccessType

	static FileAccess *create(AccessType p_access); /// Create a file access (for the current platform) this is the only portable way of accessing files.
	static FileAccess *create_for_path(const String& p_path);
	static FileAccess *open(const String& p_path, int p_mode_flags, Error *r_error=NULL); /// Create a file access (for the current platform) this is the only portable way of accessing files.
	static CreateFunc get_create_func(AccessType p_access);
	static bool exists(const String& p_name); ///< return true if a file exists
	static uint64_t get_modified_time(const String& p_file);


	static void set_backup_save(bool p_enable) { backup_save=p_enable; };
	static bool is_backup_save_enabled() { return backup_save; };

	static String get_md5(const String& p_file);

	static Vector<uint8_t> get_file_as_array(const String& p_path);


	template<class T>
	static void make_default(AccessType p_access) {

		create_func[p_access]=_create_builtin<T>;
	}

	FileAccess();
	virtual ~FileAccess(){}

};


struct FileAccessRef {

	_FORCE_INLINE_ FileAccess* operator->() {

		return f;
	}

	operator bool() const { return f!=NULL; }
	FileAccess *f;
	FileAccessRef(FileAccess *fa) { f = fa; }
	~FileAccessRef() { if (f) memdelete(f); }
};

#endif
