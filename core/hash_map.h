/*************************************************************************/
/*  hash_map.h                                                           */
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
#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "hashfuncs.h"
#include "error_macros.h"
#include "types/ustring.h"
#include "os/memory.h"
#include "list.h"


class HashMapHahserDefault {
public:
	
	static _FORCE_INLINE_ uint32_t hash(const String &p_string)  { return p_string.hash(); }
	static _FORCE_INLINE_ uint32_t hash(const char *p_cstr)  { return hash_djb2(p_cstr); }
	static _FORCE_INLINE_ uint32_t hash(const uint64_t p_int)  {
		uint64_t v=p_int;
		v = (~v) + (v << 18); // v = (v << 18) - v - 1;
		v = v ^ (v >> 31);
		v = v * 21; // v = (v + (v << 2)) + (v << 4);
		v = v ^ (v >> 11);
		v = v + (v << 6);
		v = v ^ (v >> 22);
		return (int) v;
	}
	static _FORCE_INLINE_ uint32_t hash(const int64_t p_int)  { return hash(uint64_t(p_int)); }


	static _FORCE_INLINE_ uint32_t hash(const uint32_t p_int)  { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(const int32_t p_int)  { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(const uint16_t p_int)  { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(const int16_t p_int)  { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(const uint8_t p_int)  { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(const int8_t p_int)  { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(const wchar_t p_wchar)  { return (uint32_t)p_wchar; }
//	static _FORCE_INLINE_ uint32_t hash(const void* p_ptr)  { return uint32_t(uint64_t(p_ptr))*(0x9e3779b1L); }
};

/**
 * @class HashMap
 * @author Juan Linietsky <reduzio@gmail.com> 
 * 
 * Implementation of a standard Hashing HashMap, for quick lookups of Data associated with a Key.
 * The implementation provides hashers for the default types, if you need a special kind of hasher, provide
 * your own.
 * @param TKey  Key, search is based on it, needs to be hasheable. It is unique in this container.
 * @param TData Data, data associated with the key
 * @param Hasher Hasher object, needs to provide a valid static hash function for TKey
 * @param MIN_HASH_TABLE_POWER Miminum size of the hash table, as a power of two. You rarely need to change this parameter.
 * @param RELATIONSHIP Relationship at which the hash table is resized. if amount of elements is RELATIONSHIP
 * times bigger than the hash table, table is resized to solve this condition. if RELATIONSHIP is zero, table is always MIN_HASH_TABLE_POWER.
 * 
*/

template<class TKey, class TData, class Hasher=HashMapHahserDefault,uint8_t MIN_HASH_TABLE_POWER=3,uint8_t RELATIONSHIP=8>
class HashMap {
public:	
	
	struct Pair {
			
		TKey key;
		TData data;
		
		Pair() {}
		Pair(const TKey& p_key, const TData& p_data) { key=p_key; data=p_data; }
	};
	
		
private:	
	struct Entry {
		
		uint32_t hash;
		Entry *next;
		Pair pair;
		
		Entry() { next=0; }
	};
	
	Entry **hash_table;
	uint8_t hash_table_power;
	uint32_t elements;
	
	void make_hash_table() {
		
		ERR_FAIL_COND( hash_table );
		
		
		hash_table = memnew_arr( Entry*, (1<<MIN_HASH_TABLE_POWER) );
		
		hash_table_power = MIN_HASH_TABLE_POWER;
		elements=0;
		for (int i=0;i<(1<<MIN_HASH_TABLE_POWER);i++)
			hash_table[i]=0;
	}
	
	void erase_hash_table() {
	
		ERR_FAIL_COND(elements);
		
		memdelete_arr( hash_table );
		hash_table=0;
		hash_table_power=0;		
		elements=0;	
	}	
	
	void check_hash_table() {

		int new_hash_table_power=-1;
	
		if ((int)elements > ( (1<<hash_table_power) * RELATIONSHIP ) ) {
			/* rehash up */
			new_hash_table_power=hash_table_power+1;
		
			while( (int)elements > ( (1<<new_hash_table_power) * RELATIONSHIP ) ) {
		
				new_hash_table_power++;
			}
	
		} else if ( (hash_table_power>(int)MIN_HASH_TABLE_POWER) && ((int)elements < ( (1<<(hash_table_power-1)) * RELATIONSHIP )  )  ) {
	
			/* rehash down */
			new_hash_table_power=hash_table_power-1;
		
			while( (int)elements < ( (1<<(new_hash_table_power-1)) * RELATIONSHIP ) ) {
		
				new_hash_table_power--;
			}
	
			if (new_hash_table_power<(int)MIN_HASH_TABLE_POWER)
				new_hash_table_power=MIN_HASH_TABLE_POWER;
		}
		
		
		if (new_hash_table_power==-1)
			return;
		
		Entry ** new_hash_table = memnew_arr( Entry*, (1<<new_hash_table_power) );
		if (!new_hash_table) {
	
			ERR_PRINT("Out of Memory");
			return;
		}
	
		for (int i=0;i<(1<<new_hash_table_power);i++) {
	
			new_hash_table[i]=0;
		}
	
		for (int i=0;i<(1<<hash_table_power);i++) {
	
			while( hash_table[i] ) {
		
				Entry *se=hash_table[i];
				hash_table[i]=se->next;
				int new_pos = se->hash & ((1<<new_hash_table_power)-1);
				se->next=new_hash_table[new_pos];
				new_hash_table[new_pos]=se;
			}
		
		}
	
		if (hash_table)
			memdelete_arr( hash_table );
		hash_table=new_hash_table;
		hash_table_power=new_hash_table_power;	

	}


	/* I want to have only one function.. */
	_FORCE_INLINE_ const Entry * get_entry( const TKey& p_key ) const {
		
		uint32_t hash = Hasher::hash( p_key );
		uint32_t index = hash&((1<<hash_table_power)-1);
		
		Entry *e = hash_table[index];
		
		while (e) {
			
			/* checking hash first avoids comparing key, which may take longer */
			if (e->hash == hash && e->pair.key == p_key ) {
				
				/* the pair exists in this hashtable, so just update data */
				return e;
			}
			
			e=e->next;
		}
				
		return NULL;
	}
	
	Entry * create_entry(const TKey& p_key) {
				
		/* if entry doesn't exist, create it */
		Entry *e = memnew( Entry );
		ERR_FAIL_COND_V(!e,NULL); /* out of memory */
		uint32_t hash = Hasher::hash( p_key );
		uint32_t index = hash&((1<<hash_table_power)-1);
		e->next = hash_table[index];	
		e->hash = hash;
		e->pair.key=p_key;
				
		hash_table[index]=e;
		elements++;
		
		return e;
	}
	
	
	void copy_from(const HashMap& p_t) {
		
		if (&p_t==this)
			return; /* much less bother with that */
		
		clear();
		
		if (!p_t.hash_table || p_t.hash_table_power==0)
			return; /* not copying from empty table */
		
		hash_table = memnew_arr(Entry*,1<<p_t.hash_table_power);
		hash_table_power=p_t.hash_table_power;
		elements=p_t.elements;
		
		for (int i=0;i<( 1<<p_t.hash_table_power );i++) {
			
			hash_table[i]=NULL;
			/* elements will be in the reverse order, but it doesn't matter */
			
			const Entry *e = p_t.hash_table[i];
			
			while(e) {
				
				Entry *le = memnew( Entry ); /* local entry */
				
				*le=*e; /* copy data */
				
				/* add to list and reassign pointers */
				le->next=hash_table[i];
				hash_table[i]=le;
				
				e=e->next;
			}
			
			
		}
		
		
	}
public:
	
	
	void set( const TKey& p_key, const TData& p_data ) {
	
		set( Pair( p_key, p_data ) );
		
	}
	
	void set( const Pair& p_pair ) {

		Entry *e=NULL;
		if (!hash_table)
			make_hash_table(); // if no table, make one
		else
			e = const_cast<Entry*>( get_entry(p_pair.key) );
		
		/* if we made it up to here, the pair doesn't exist, create and assign */
		
		if (!e) {
			
			e=create_entry(p_pair.key);
			if (!e)
				return;
			check_hash_table(); // perform mantenience routine
		}
		
		e->pair.data = p_pair.data;
		
	}
	
	
	bool has( const TKey& p_key ) const {
		
		return getptr(p_key)!=NULL;
	}
	
	/**
	 * Get a key from data, return a const reference. 
	 * WARNING: this doesn't check errors, use either getptr and check NULL, or check
	 * first with has(key)
	 */
	
	const TData& get( const TKey& p_key ) const {
		
		const TData* res = getptr(p_key);
		ERR_FAIL_COND_V(!res,*res);
		return *res;
	}
	
	TData& get( const TKey& p_key )  {
		
		TData* res = getptr(p_key);
		ERR_FAIL_COND_V(!res,*res);
		return *res;
	}
	
	/**
	 * Same as get, except it can return NULL when item was not found.
	 * This is mainly used for speed purposes.
	 */

	
	_FORCE_INLINE_  TData* getptr( const TKey& p_key ) {
		
		if (!hash_table)
			return NULL;
		
		Entry *e=const_cast<Entry*>(get_entry(p_key ));
		
		if (e)
			return &e->pair.data;
		
		return NULL;

	}

	_FORCE_INLINE_  const TData* getptr( const TKey& p_key ) const {

		if (!hash_table)
			return NULL;

		const Entry *e=const_cast<Entry*>(get_entry(p_key ));

		if (e)
			return &e->pair.data;

		return NULL;

	}

	/**
	 * Same as get, except it can return NULL when item was not found.
	 * This version is custom, will take a hash and a custom key (that should support operator==()
	 */

	template<class C>
	_FORCE_INLINE_ TData* custom_getptr( C p_custom_key,uint32_t p_custom_hash )  {

		if (!hash_table)
			return NULL;

		uint32_t hash = p_custom_hash;
		uint32_t index = hash&((1<<hash_table_power)-1);

		Entry *e = hash_table[index];

		while (e) {

			/* checking hash first avoids comparing key, which may take longer */
			if (e->hash == hash && e->pair.key == p_custom_key ) {

				/* the pair exists in this hashtable, so just update data */
				return &e->pair.data;
			}

			e=e->next;
		}

		return NULL;
	}

	template<class C>
	_FORCE_INLINE_ const TData* custom_getptr( C p_custom_key,uint32_t p_custom_hash ) const {

		if (!hash_table)
			return NULL;

		uint32_t hash = p_custom_hash;
		uint32_t index = hash&((1<<hash_table_power)-1);

		const Entry *e = hash_table[index];

		while (e) {

			/* checking hash first avoids comparing key, which may take longer */
			if (e->hash == hash && e->pair.key == p_custom_key ) {

				/* the pair exists in this hashtable, so just update data */
				return &e->pair.data;
			}

			e=e->next;
		}

		return NULL;
	}


	/**
	 * Erase an item, return true if erasing was succesful
	 */
	
	bool erase( const TKey& p_key ) {
		
		if (!hash_table)
			return false;
		
		uint32_t hash = Hasher::hash( p_key );
		uint32_t index = hash&((1<<hash_table_power)-1);
		
		
		Entry *e = hash_table[index];
		Entry *p=NULL;
		while (e) {
			
			/* checking hash first avoids comparing key, which may take longer */
			if (e->hash == hash && e->pair.key == p_key ) {
				
				if (p) {
					
					p->next=e->next;
				} else { 
					//begin of list
					hash_table[index]=e->next;
				}
				
				memdelete(e);
				elements--;
				
				if (elements==0)
					erase_hash_table();
				else
					check_hash_table();		
				return true;
			}
			
			p=e;
			e=e->next;
		}
		
		
		return false;
		
	}
	
	inline const TData& operator[](const TKey& p_key) const { //constref
		
		return get(p_key);
	}
	inline TData& operator[](const TKey& p_key ) { //assignment
		
		Entry *e=NULL;
		if (!hash_table)
			make_hash_table(); // if no table, make one
		else
			e = const_cast<Entry*>( get_entry(p_key) );
		
		/* if we made it up to here, the pair doesn't exist, create */
		if (!e) {
			
			e=create_entry(p_key);
			if (!e)
				return *(TData*)NULL; /* panic! */
			check_hash_table(); // perform mantenience routine
		}
		
		return e->pair.data;
		
	}
	
	/** 
	 * Get the next key to p_key, and the first key if p_key is null. 
	 * Returns a pointer to the next key if found, NULL otherwise.
	 * Adding/Removing elements while iterating will, of course, have unexpected results, don't do it.
	 * 
	 * Example:
	 * 
	 * 	const TKey *k=NULL;
	 * 
	 * 	while( (k=table.next(k)) ) {
	 * 
	 * 		print( *k );
	 * 	}
         *	
	*/
	const TKey* next(const TKey* p_key) const {
		
		if (!hash_table) return NULL;
		
		if (!p_key) { /* get the first key */
			
			for (int i=0;i<(1<<hash_table_power);i++) {
				
				if (hash_table[i]) {
					return &hash_table[i]->pair.key;
				}
			}

		} else { /* get the next key */
			
			const Entry *e = get_entry( *p_key );
			ERR_FAIL_COND_V( !e, NULL ); /* invalid key supplied */
			
			if (e->next) {
				/* if there is a "next" in the list, return that */
				return &e->next->pair.key;
			} else {
				/* go to next entries */
				uint32_t index = e->hash&((1<<hash_table_power)-1);
				index++;
				for (int i=index;i<(1<<hash_table_power);i++) {
				
					if (hash_table[i]) {
						return &hash_table[i]->pair.key;
					}
				}
			}
				
			/* nothing found, was at end */
			
		}
		
		
		return NULL; /* nothing found */
	}
	
	inline unsigned int size() const {
		
		return elements;
	}
	
	inline bool empty() const {
		
		return elements==0;
	}
	
	void clear() {
		
		/* clean up */
		if (hash_table) {
			for (int i=0;i<(1<<hash_table_power);i++) {
				
				while (hash_table[i]) {
								
					Entry *e=hash_table[i];
					hash_table[i]=e->next;
					memdelete( e );
				}
			}
		
			memdelete_arr( hash_table );
		}
		
		hash_table=0;
		hash_table_power=0;
		elements=0;		
	}
	
	
	void operator=(const HashMap& p_table) {
		
		copy_from(p_table);
	}
	
	HashMap() { 
		hash_table=NULL;
		elements=0;
		hash_table_power=0;
	}

	void get_key_list(List<TKey> *p_keys) const {
		if (!hash_table)
			return;
		for(int i=0;i<(1<<hash_table_power);i++) {

			Entry *e=hash_table[i];
			while(e) {
				p_keys->push_back(e->pair.key);
				e=e->next;
			}
		}
	}

	HashMap(const HashMap& p_table) { 
	
		hash_table=NULL;
		elements=0;
		hash_table_power=0;
		
		copy_from(p_table);
		
	}
	
	~HashMap() {
		
		clear();
	}

};

#endif
