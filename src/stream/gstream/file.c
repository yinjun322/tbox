/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2012, ruki All rights reserved.
 *
 * @author		ruki
 * @file		file.c
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../../asio/asio.h"
#include "../../string/string.h"
#include "../../platform/platform.h"

/* ///////////////////////////////////////////////////////////////////////
 * types
 */

// the file stream type
typedef struct __tb_gstream_file_t
{
	// the base
	tb_gstream_t 		base;

	// the file handle
	tb_handle_t 		file;

	// the file size
	tb_hize_t 			size;

	// the file bref
	tb_size_t 			bref;

	// the file mode
	tb_size_t 			mode;

}tb_gstream_file_t;


/* ///////////////////////////////////////////////////////////////////////
 * details
 */
static __tb_inline__ tb_gstream_file_t* tb_gstream_file_cast(tb_gstream_t* gst)
{
	tb_assert_and_check_return_val(gst && gst->type == TB_GSTREAM_TYPE_FILE, tb_null);
	return (tb_gstream_file_t*)gst;
}
static tb_long_t tb_gstream_file_aopen(tb_gstream_t* gst)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && !fst->file, -1);

	// no reference?
	tb_check_return_val(!(fst->file && fst->bref), 1);

	// url
	tb_char_t const* url = tb_url_get(&gst->url);
	tb_assert_and_check_return_val(url, -1);

	// open file
	fst->file = tb_file_init(url, fst->mode);
	tb_assert_and_check_return_val(fst->file, -1);

	// ok
	return 1;
}
static tb_long_t tb_gstream_file_aclose(tb_gstream_t* gst)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst, -1);

	if (fst->file)
	{
		// close file
		if (!fst->bref) if (!tb_file_exit(fst->file)) return 0;

		// reset
		fst->file = tb_null;
		fst->bref = 0;
	}

	// ok
	return 1;
}
static tb_long_t tb_gstream_file_aread(tb_gstream_t* gst, tb_byte_t* data, tb_size_t size, tb_bool_t sync)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && fst->file, -1);

	// check
	tb_check_return_val(data, -1);
	tb_check_return_val(size, 0);

	// read 
	return tb_file_read(fst->file, data, size);
}
static tb_long_t tb_gstream_file_awrit(tb_gstream_t* gst, tb_byte_t* data, tb_size_t size, tb_bool_t sync)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && fst->file, -1);

	// has data
	if (data)
	{
		// check
		tb_check_return_val(size, 0);

		// writ
		return tb_file_writ(fst->file, data, size);
	}
	
end:
	// sync data
	if (sync) if (!tb_file_sync(fst->file)) return -1;

	// end?
	return -1;
}
static tb_long_t tb_gstream_file_aseek(tb_gstream_t* gst, tb_hize_t offset)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && fst->file, -1);

	// seek
	return (tb_file_seek(fst->file, offset, TB_FILE_SEEK_BEG) == offset)? 1 : -1;
}
static tb_hize_t tb_gstream_file_size(tb_gstream_t* gst)
{	
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && fst->file, 0);
	return tb_file_size(fst->file);
}
static tb_long_t tb_gstream_file_wait(tb_gstream_t* gst, tb_size_t wait, tb_long_t timeout)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst && fst->file, -1);

	// ok?
	return wait;
}
static tb_bool_t tb_gstream_file_ctrl(tb_gstream_t* gst, tb_size_t ctrl, tb_va_list_t args)
{
	tb_gstream_file_t* fst = tb_gstream_file_cast(gst);
	tb_assert_and_check_return_val(fst, tb_false);

	switch (ctrl)
	{
	case TB_GSTREAM_CTRL_FILE_SET_MODE:
		fst->mode = (tb_size_t)tb_va_arg(args, tb_size_t);
		return tb_true;
	case TB_GSTREAM_CTRL_FILE_SET_HANDLE:
		{
			tb_handle_t handle = (tb_handle_t)tb_va_arg(args, tb_handle_t);
			fst->file = handle;
			fst->bref = handle? 1 : 0;
			return tb_true;
		}
	case TB_GSTREAM_CTRL_FILE_GET_HANDLE:
		{
			tb_handle_t* phandle = (tb_handle_t)tb_va_arg(args, tb_handle_t*);
			tb_assert_and_check_return_val(phandle, tb_false);
			*phandle = fst->file;
			return tb_true;
		}
	default:
		break;
	}
	return tb_false;
}
/* ///////////////////////////////////////////////////////////////////////
 * interfaces
 */

tb_gstream_t* tb_gstream_init_file()
{
	// make stream
	tb_gstream_t* gst = (tb_gstream_t*)tb_malloc0(sizeof(tb_gstream_file_t));
	tb_assert_and_check_return_val(gst, tb_null);

	// init base
	if (!tb_gstream_init(gst)) goto fail;

	// init stream
	tb_gstream_file_t* fst = (tb_gstream_file_t*)gst;
	gst->type 	= TB_GSTREAM_TYPE_FILE;
	gst->aopen 	= tb_gstream_file_aopen;
	gst->aclose = tb_gstream_file_aclose;
	gst->aread 	= tb_gstream_file_aread;
	gst->awrit 	= tb_gstream_file_awrit;
	gst->aseek 	= tb_gstream_file_aseek;
	gst->size 	= tb_gstream_file_size;
	gst->wait 	= tb_gstream_file_wait;
	gst->ctrl 	= tb_gstream_file_ctrl;
	fst->file 	= tb_null;
	fst->mode 	= TB_FILE_MODE_RO | TB_FILE_MODE_BINARY;

	// resize file cache
	if (!tb_gstream_ctrl(gst, TB_GSTREAM_CTRL_SET_CACHE, TB_FILE_DIRECT_CSIZE)) goto fail;

	// ok
	return gst;

fail:
	if (gst) tb_gstream_exit(gst);
	return tb_null;
}

tb_gstream_t* tb_gstream_init_from_file(tb_char_t const* path)
{
	tb_assert_and_check_return_val(path, tb_null);

	// init file stream
	tb_gstream_t* gst = tb_gstream_init_file();
	tb_assert_and_check_return_val(gst, tb_null);

	// set path
	if (!tb_gstream_ctrl(gst, TB_GSTREAM_CTRL_SET_URL, path)) goto fail;
	
	return gst;

fail:
	if (gst) tb_gstream_exit(gst);
	return tb_null;
}
