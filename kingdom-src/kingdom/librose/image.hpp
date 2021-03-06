/* $Id: image.hpp 47608 2010-11-21 01:56:29Z shadowmaster $ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "map_location.hpp"
#include "sdl_utils.hpp"
#include "terrain_translation.hpp"

///this module manages the cache of images. With an image name, you can get
///the surface corresponding to that image.
//
namespace image {
extern int tile_size;

template<typename T>
class cache_type;

//a generic image locator. Abstracts the location of an image.
class locator
{
public:
	enum type { NONE, FILE, SUB_FILE };
private:
	// Called by each constructor after actual construction to
	// initialize the index_ field
	void parse_arguments();
	struct value {
		value();
		value(const value &a);
		value(const char *filename);
		value(const std::string& filename);
		value(const std::string& filename, const std::string& modifications);
		value(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications);

		type type_;
		std::string filename_;
		map_location loc_;
		std::string modifications_;
		int center_x_;
		int center_y_;
	};

	friend size_t hash_value(const value&);
	friend size_t hash_value1(const value&);

public:

	// Constructing locators is somewhat slow, accessing image
	// through locators is fast. The idea is that calling functions
	// should store locators, and not strings to construct locators
	// (the second will work, of course, but will be slower)
	locator();
	locator(const locator &a, const std::string &mods ="");
	locator(const char *filename);
	locator(const std::string& filename);
	locator(const std::string& filename, const std::string& modifications);
	locator(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications="");

	locator& operator=(const locator &a);

	bool operator==(const locator &a)const;

	const std::string &get_filename() const { return val_.filename_; }
	const map_location& get_loc() const { return val_.loc_ ; }
	int get_center_x() const { return val_.center_x_; }
	int get_center_y() const { return val_.center_y_; }
	const std::string& get_modifications() const {return val_.modifications_;}
	type get_type() const { return val_.type_; };

	// returns true if the locator does not correspond to any
	// actual image
	bool is_void() const { return val_.type_ == NONE; }

	/**
	 * Tests whether the file the locater points at exists.
	 *
	 * is_void doesn't seem to work before the image is loaded and also in
	 * debug mode a placeholder is returned. So it's not possible to test
	 * for the existence of a file. So this function does that. (Note it
	 * tests for existence not whether or not it's a valid image.)
	 *
	 * @return                Whether or not the file exists.
	 */
	bool file_exists();

	// loads the image it is pointing to from the disk
	surface load_from_disk() const;

	template <typename T>
	int in_cache(cache_type<T> &cache) const;
	template <typename T>
	const T &locate_in_cache(cache_type<T> &cache, int index) const;
	template <typename T>
	void add_to_cache(cache_type<T> &cache, const T &data) const;

private:

	surface load_image_file() const;
	surface load_image_sub_file() const;

	value val_;
	size_t hash_;
	size_t hash1_;
};

size_t hash_value(const locator::value&);

extern std::string terrain_prefix;
extern surface mask_surf;
extern locator grid_top;
extern locator grid_bottom;
extern const int scale_ratio;
extern int scale_ratio_w;
extern int scale_ratio_h;
typedef void (* fadjust_x_y)(int& x, int& y);
extern fadjust_x_y minimap_tile_dst;

void switch_tile(const std::string& tile);
class ttile_switch_lock
{
public:
	ttile_switch_lock(const std::string& tile);
	~ttile_switch_lock();

private:
	std::string terrain_prefix_;
	surface surf_;
	locator grid_top_;
	locator grid_bottom_;
	int scale_ratio_w_;
	int scale_ratio_h_;
	fadjust_x_y minimap_tile_dst_;
};

typedef cache_type<surface> image_cache;
typedef cache_type<bool> bool_cache;
typedef std::map<t_translation::t_terrain, surface> mini_terrain_cache_map;
extern mini_terrain_cache_map mini_terrain_cache;
extern mini_terrain_cache_map mini_fogged_terrain_cache;

void flush_cache(bool force = false);

///the image manager is responsible for setting up images, and destroying
///all images when the program exits. It should probably
///be created once for the life of the program
struct manager
{
	manager();
	~manager();
};

///will make all scaled images have these rgb values added to all
///their pixels. i.e. add a certain color hint to images. useful
///for representing day/night. Invalidates all scaled images.
void set_color_adjustment(int r, int g, int b);

class color_adjustment_resetter
{
	public:
		color_adjustment_resetter();
		void reset();
	private:
		int r_, g_, b_;
};

///set the team colors used by the TC image modification
///use a vector with one string for each team
///using NULL will reset to default TC
void set_team_colors(const std::vector<std::string>* colors = NULL);
std::vector<std::string>& get_team_colors();

///sets the pixel format used by the images. Is called every time the
///video mode changes. Invalidates all images.
void set_pixel_format(SDL_PixelFormat* format);

///sets the amount scaled images should be scaled. Invalidates all
///scaled images.
void set_zoom(int zoom);

/// UNSCALED : image will be drawn "as is" without changing size, even in case of redraw
/// SCALED_TO_ZOOM : image will be scaled taking zoom into account
/// HEXED : the hex mask is applied on the image
/// SCALED_TO_HEX : image will be scaled to fit into a hex, taking zoom into account
/// TOD_COLORED : same as SCALED_TO_HEX but ToD coloring is also applied
/// BRIGHTENED  : same as TOD_COLORED but also brightened
/// SEMI_BRIGHTENED  : same as TOD_COLORED but also semi-brightened
enum TYPE { UNSCALED, SCALED_TO_ZOOM, HEXED, SCALED_TO_HEX, TOD_COLORED, BRIGHTENED, SEMI_BRIGHTENED};

///function to get the surface corresponding to an image.
///note that this surface must be freed by the user by calling
///SDL_FreeSurface()
surface get_image(const locator& i_locator, TYPE type=UNSCALED);

///function to get the standard hex mask
surface get_hexmask();

///function to get the hexed image
surface get_hexed(const locator& i_locator);

///function to check if an image fit into an hex
///return false if the image has not the standard size.
bool is_in_hex(const locator& i_locator);

///function to check if an image is empty after hex cut
///should be only used on terrain image (cache the hex cut version)
bool is_empty_hex(const locator& i_locator);

///function to reverse an image. The image MUST have originally been returned from
///an image:: function. Returned images have the same semantics as for get_image()
///and must be freed using SDL_FreeSurface()
surface reverse_image(const surface &surf);

///returns true if the given image actually exists, without loading it.
bool exists(const locator& i_locator);

/// precache the existence of files in the subdir (ex: "terrain/")
void precache_file_existence(const std::string& subdir = "");
bool precached_file_exists(const std::string& file);
}

#endif

