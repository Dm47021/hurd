/* Store wire decoding

   Copyright (C) 1996 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This file is part of the GNU Hurd.

   The GNU Hurd is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The GNU Hurd is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <string.h>

#include "store.h"

/* Decode ENC, either returning a new store in STORE, or an error.  If
   nothing else is to be done with ENC, its contents may then be freed using
   store_enc_dealloc.  */
error_t
store_decode (struct store_enc *enc, struct store **store)
{
  if (enc->cur_ints >= enc->ints_len)
    /* The first int should always be the type.  */
    return EINVAL;

  switch (enc->ints[enc->cur_ints])
    {
    case STORAGE_HURD_FILE:
    case STORAGE_DEVICE:
    case STORAGE_TASK:
    case STORAGE_MEMORY:
      return store_default_leaf_decode (enc, store);

    case STORAGE_ILEAVE:
      return store_ileave_decode (enc, store);
    case STORAGE_CONCAT:
      return store_concat_decode (enc, store);
    case STORAGE_LAYER:
      return store_layer_decode (enc, store);
    case STORAGE_NULL:
      return store_null_decode (enc, store);

    default:
      return EINVAL;
    }
}
