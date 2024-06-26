/* mRss - Copyright (C) 2005-2006 bakunin - Andrea Marchesini 
 *                                    <bakunin@autistici.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
# error Use configure; make; make install
#endif

#include "mrss.h"
#include "mrss_internal.h"

static mrss_error_t __mrss_set_channel (mrss_t *, va_list);
static mrss_error_t __mrss_set_item (mrss_item_t *, va_list);
static mrss_error_t __mrss_set_hour (mrss_hour_t *, va_list);
static mrss_error_t __mrss_set_day (mrss_day_t *, va_list);
static mrss_error_t __mrss_set_category (mrss_category_t *, va_list);

static mrss_error_t __mrss_get_channel (mrss_t *, va_list);
static mrss_error_t __mrss_get_item (mrss_item_t *, va_list);
static mrss_error_t __mrss_get_hour (mrss_hour_t *, va_list);
static mrss_error_t __mrss_get_day (mrss_day_t *, va_list);
static mrss_error_t __mrss_get_category (mrss_category_t *, va_list);

static mrss_error_t __mrss_new_subdata_channel (mrss_t *, mrss_element_t,
						mrss_generic_t);
static mrss_error_t __mrss_new_subdata_item (mrss_item_t *, mrss_element_t,
					     mrss_generic_t);

static mrss_error_t __mrss_remove_subdata_channel (mrss_t *, mrss_generic_t);
static mrss_error_t __mrss_remove_subdata_item (mrss_item_t *,
						mrss_generic_t);

#define __MRSS_SET_STRING( x ) \
		if(x) free(x); \
		if(value && !(x=strdup(value))) return MRSS_ERR_POSIX; \
		else if(!value) x=NULL;

#define __MRSS_SET_INTEGER( x ) \
		x=value_int;

#define __MRSS_GET_STRING( x ) \
		string = (char **)value; \
		if (!x) *string = NULL; \
		else if(!(*string = strdup(x))) return MRSS_ERR_POSIX;

#define __MRSS_GET_INTEGER( x ) \
		integer=(int *)value; \
		*integer=x;

mrss_error_t
mrss_new (mrss_t ** data)
{
  int allocated;

  if (!data)
    return MRSS_ERR_DATA;

  if (!*data)
    {
      if (!(*data = (mrss_t *) malloc (sizeof (mrss_t))))
	return MRSS_ERR_POSIX;

      allocated = 1;
    }
  else
    allocated = 0;

  memset (*data, 0, sizeof (mrss_t));
  (*data)->element = MRSS_ELEMENT_CHANNEL;
  (*data)->allocated = allocated;

  return MRSS_OK;
}

mrss_error_t
mrss_set (mrss_generic_t data, ...)
{
  va_list va;
  mrss_error_t err;
  mrss_t *tmp;

  if (!data)
    return MRSS_ERR_DATA;

  va_start (va, data);

  tmp = (mrss_t *) data;
  switch (tmp->element)
    {
    case MRSS_ELEMENT_CHANNEL:
      err = __mrss_set_channel ((mrss_t *) data, va);
      break;

    case MRSS_ELEMENT_ITEM:
      err = __mrss_set_item ((mrss_item_t *) data, va);
      break;

    case MRSS_ELEMENT_SKIPHOURS:
      err = __mrss_set_hour ((mrss_hour_t *) data, va);
      break;

    case MRSS_ELEMENT_SKIPDAYS:
      err = __mrss_set_day ((mrss_day_t *) data, va);
      break;

    case MRSS_ELEMENT_CATEGORY:
      err = __mrss_set_category ((mrss_category_t *) data, va);
      break;

    default:
      err = MRSS_ERR_DATA;
      break;
    }

  va_end (va);
  return err;
}

static mrss_error_t
__mrss_set_channel (mrss_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  int value_int;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      switch (flag)
	{
	case MRSS_FLAG_VERSION:
          value = va_arg (va, void *);
	  if ((mrss_version_t) value != MRSS_VERSION_0_91 &&
	      (mrss_version_t) value != MRSS_VERSION_0_92 &&
	      (mrss_version_t) value != MRSS_VERSION_2_0)
	    return MRSS_ERR_DATA;

	  data->version = (mrss_version_t) value;
	  break;

	case MRSS_FLAG_TITLE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->title);
	  break;

	case MRSS_FLAG_DESCRIPTION:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->description);
	  break;

	case MRSS_FLAG_LINK:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->link);
	  break;

	case MRSS_FLAG_LANGUAGE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->language);
	  break;

	case MRSS_FLAG_RATING:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->rating);
	  break;

	case MRSS_FLAG_COPYRIGHT:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->copyright);
	  break;

	case MRSS_FLAG_PUBDATE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->pubDate);
	  break;

	case MRSS_FLAG_LASTBUILDDATE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->lastBuildDate);
	  break;

	case MRSS_FLAG_DOCS:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->docs);
	  break;

	case MRSS_FLAG_MANAGINGEDITOR:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->managingeditor);
	  break;

	case MRSS_FLAG_WEBMASTER:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->webMaster);
	  break;

	case MRSS_FLAG_GENERATOR:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->generator);
	  break;

	case MRSS_FLAG_TTL:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->ttl);
	  break;

	case MRSS_FLAG_ABOUT:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->about);
	  break;

	case MRSS_FLAG_IMAGE_TITLE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->image_title);
	  break;

	case MRSS_FLAG_IMAGE_URL:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->image_url);
	  break;

	case MRSS_FLAG_IMAGE_LINK:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->image_link);
	  break;

	case MRSS_FLAG_IMAGE_WIDTH:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->image_width);
	  break;

	case MRSS_FLAG_IMAGE_HEIGHT:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->image_height);
	  break;

	case MRSS_FLAG_IMAGE_DESCRIPTION:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->image_description);
	  break;

	case MRSS_FLAG_TEXTINPUT_TITLE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->textinput_title);
	  break;

	case MRSS_FLAG_TEXTINPUT_DESCRIPTION:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->textinput_description);
	  break;

	case MRSS_FLAG_TEXTINPUT_NAME:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->textinput_name);
	  break;

	case MRSS_FLAG_TEXTINPUT_LINK:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->textinput_link);
	  break;

	case MRSS_FLAG_CLOUD:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->cloud);
	  break;

	case MRSS_FLAG_CLOUD_DOMAIN:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->cloud_domain);
	  break;

	case MRSS_FLAG_CLOUD_PORT:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->cloud_port);
	  break;

	case MRSS_FLAG_CLOUD_PATH:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->cloud_path);
	  break;

	case MRSS_FLAG_CLOUD_REGISTERPROCEDURE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->cloud_registerProcedure);
	  break;

	case MRSS_FLAG_CLOUD_PROTOCOL:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->cloud_protocol);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_set_hour (mrss_hour_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_HOUR:
	  __MRSS_SET_STRING (data->hour);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_set_day (mrss_day_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_DAY:
	  __MRSS_SET_STRING (data->day);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_set_category (mrss_category_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_CATEGORY:
	  __MRSS_SET_STRING (data->category);
	  break;

	case MRSS_FLAG_CATEGORY_DOMAIN:
	  __MRSS_SET_STRING (data->domain);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_set_item (mrss_item_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  int value_int;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);
      switch (flag)
	{
	case MRSS_FLAG_ITEM_TITLE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->title);
	  break;

	case MRSS_FLAG_ITEM_LINK:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->link);
	  break;

	case MRSS_FLAG_ITEM_DESCRIPTION:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->description);
	  break;

	case MRSS_FLAG_ITEM_AUTHOR:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->author);
	  break;

	case MRSS_FLAG_ITEM_COMMENTS:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->comments);
	  break;

	case MRSS_FLAG_ITEM_PUBDATE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->pubDate);
	  break;

	case MRSS_FLAG_ITEM_GUID:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->guid);
	  break;

	case MRSS_FLAG_ITEM_GUID_ISPERMALINK:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->guid_isPermaLink);
	  break;

	case MRSS_FLAG_ITEM_SOURCE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->source);
	  break;

	case MRSS_FLAG_ITEM_SOURCE_URL:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->source_url);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->enclosure);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_URL:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->enclosure_url);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_LENGTH:
          value_int = va_arg (va, int);
	  __MRSS_SET_INTEGER (data->enclosure_length);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_TYPE:
          value = va_arg (va, void *);
	  __MRSS_SET_STRING (data->enclosure_type);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

mrss_error_t
mrss_get (mrss_generic_t data, ...)
{
  va_list va;
  mrss_error_t err;
  mrss_t *tmp;

  if (!data)
    return MRSS_ERR_DATA;

  va_start (va, data);

  tmp = (mrss_t *) data;
  switch (tmp->element)
    {
    case MRSS_ELEMENT_CHANNEL:
      err = __mrss_get_channel ((mrss_t *) data, va);
      break;

    case MRSS_ELEMENT_ITEM:
      err = __mrss_get_item ((mrss_item_t *) data, va);
      break;

    case MRSS_ELEMENT_SKIPHOURS:
      err = __mrss_get_hour ((mrss_hour_t *) data, va);
      break;

    case MRSS_ELEMENT_SKIPDAYS:
      err = __mrss_get_day ((mrss_day_t *) data, va);
      break;

    case MRSS_ELEMENT_CATEGORY:
      err = __mrss_get_category ((mrss_category_t *) data, va);
      break;

    default:
      err = MRSS_ERR_DATA;
      break;
    }

  va_end (va);
  return err;
}

static mrss_error_t
__mrss_get_channel (mrss_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  int *integer;
  mrss_version_t *version;
  char **string;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_VERSION:
	  version = value;
	  *version = data->version;
	  break;

	case MRSS_FLAG_TITLE:
	  __MRSS_GET_STRING (data->title);
	  break;

	case MRSS_FLAG_DESCRIPTION:
	  __MRSS_GET_STRING (data->description);
	  break;

	case MRSS_FLAG_LINK:
	  __MRSS_GET_STRING (data->link);
	  break;

	case MRSS_FLAG_LANGUAGE:
	  __MRSS_GET_STRING (data->language);
	  break;

	case MRSS_FLAG_RATING:
	  __MRSS_GET_STRING (data->rating);
	  break;

	case MRSS_FLAG_COPYRIGHT:
	  __MRSS_GET_STRING (data->copyright);
	  break;

	case MRSS_FLAG_PUBDATE:
	  __MRSS_GET_STRING (data->pubDate);
	  break;

	case MRSS_FLAG_LASTBUILDDATE:
	  __MRSS_GET_STRING (data->lastBuildDate);
	  break;

	case MRSS_FLAG_DOCS:
	  __MRSS_GET_STRING (data->docs);
	  break;

	case MRSS_FLAG_MANAGINGEDITOR:
	  __MRSS_GET_STRING (data->managingeditor);
	  break;

	case MRSS_FLAG_WEBMASTER:
	  __MRSS_GET_STRING (data->webMaster);
	  break;

	case MRSS_FLAG_GENERATOR:
	  __MRSS_GET_STRING (data->generator);
	  break;

	case MRSS_FLAG_TTL:
	  __MRSS_GET_INTEGER (data->ttl);
	  break;

	case MRSS_FLAG_ABOUT:
	  __MRSS_GET_STRING (data->about);
	  break;

	case MRSS_FLAG_IMAGE_TITLE:
	  __MRSS_GET_STRING (data->image_title);
	  break;

	case MRSS_FLAG_IMAGE_URL:
	  __MRSS_GET_STRING (data->image_url);
	  break;

	case MRSS_FLAG_IMAGE_LINK:
	  __MRSS_GET_STRING (data->image_link);
	  break;

	case MRSS_FLAG_IMAGE_WIDTH:
	  __MRSS_GET_INTEGER (data->image_width);
	  break;

	case MRSS_FLAG_IMAGE_HEIGHT:
	  __MRSS_GET_INTEGER (data->image_height);
	  break;

	case MRSS_FLAG_IMAGE_DESCRIPTION:
	  __MRSS_GET_STRING (data->image_description);
	  break;

	case MRSS_FLAG_TEXTINPUT_TITLE:
	  __MRSS_GET_STRING (data->textinput_title);
	  break;

	case MRSS_FLAG_TEXTINPUT_DESCRIPTION:
	  __MRSS_GET_STRING (data->textinput_description);
	  break;

	case MRSS_FLAG_TEXTINPUT_NAME:
	  __MRSS_GET_STRING (data->textinput_name);
	  break;

	case MRSS_FLAG_TEXTINPUT_LINK:
	  __MRSS_GET_STRING (data->textinput_link);
	  break;

	case MRSS_FLAG_CLOUD:
	  __MRSS_GET_STRING (data->cloud);
	  break;

	case MRSS_FLAG_CLOUD_DOMAIN:
	  __MRSS_GET_STRING (data->cloud_domain);
	  break;

	case MRSS_FLAG_CLOUD_PORT:
	  __MRSS_GET_INTEGER (data->cloud_port);
	  break;

	case MRSS_FLAG_CLOUD_PATH:
	  __MRSS_GET_STRING (data->cloud_path);
	  break;

	case MRSS_FLAG_CLOUD_REGISTERPROCEDURE:
	  __MRSS_GET_STRING (data->cloud_registerProcedure);
	  break;

	case MRSS_FLAG_CLOUD_PROTOCOL:
	  __MRSS_GET_STRING (data->cloud_protocol);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_get_hour (mrss_hour_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  char **string;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_HOUR:
	  __MRSS_GET_STRING (data->hour);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_get_day (mrss_day_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  char **string;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_DAY:
	  __MRSS_GET_STRING (data->day);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_get_category (mrss_category_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  char **string;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_CATEGORY:
	  __MRSS_GET_STRING (data->category);
	  break;

	case MRSS_FLAG_CATEGORY_DOMAIN:
	  __MRSS_GET_STRING (data->domain);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_get_item (mrss_item_t * data, va_list va)
{
  mrss_flag_t flag;
  void *value;
  char **string;
  int *integer;

  while ((flag = va_arg (va, mrss_flag_t)))
    {
      value = va_arg (va, void *);

      switch (flag)
	{
	case MRSS_FLAG_ITEM_TITLE:
	  __MRSS_GET_STRING (data->title);
	  break;

	case MRSS_FLAG_ITEM_LINK:
	  __MRSS_GET_STRING (data->link);
	  break;

	case MRSS_FLAG_ITEM_DESCRIPTION:
	  __MRSS_GET_STRING (data->description);
	  break;

	case MRSS_FLAG_ITEM_AUTHOR:
	  __MRSS_GET_STRING (data->author);
	  break;

	case MRSS_FLAG_ITEM_COMMENTS:
	  __MRSS_GET_STRING (data->comments);
	  break;

	case MRSS_FLAG_ITEM_PUBDATE:
	  __MRSS_GET_STRING (data->pubDate);
	  break;

	case MRSS_FLAG_ITEM_GUID:
	  __MRSS_GET_STRING (data->guid);
	  break;

	case MRSS_FLAG_ITEM_GUID_ISPERMALINK:
	  __MRSS_GET_INTEGER (data->guid_isPermaLink);
	  break;

	case MRSS_FLAG_ITEM_SOURCE:
	  __MRSS_GET_STRING (data->source);
	  break;

	case MRSS_FLAG_ITEM_SOURCE_URL:
	  __MRSS_GET_STRING (data->source_url);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE:
	  __MRSS_GET_STRING (data->enclosure);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_URL:
	  __MRSS_GET_STRING (data->enclosure_url);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_LENGTH:
	  __MRSS_GET_INTEGER (data->enclosure_length);
	  break;

	case MRSS_FLAG_ITEM_ENCLOSURE_TYPE:
	  __MRSS_GET_STRING (data->enclosure_type);
	  break;

	default:
	  return MRSS_ERR_DATA;
	}
    }

  return MRSS_OK;
}

mrss_error_t
mrss_new_subdata (mrss_generic_t data, mrss_element_t element,
		  mrss_generic_t new)
{
  mrss_t *tmp;

  if (!data || !new)
    return MRSS_ERR_DATA;

  tmp = (mrss_t *) data;

  switch (tmp->element)
    {
    case MRSS_ELEMENT_CHANNEL:
      return __mrss_new_subdata_channel ((mrss_t *) data, element, new);
    case MRSS_ELEMENT_ITEM:
      return __mrss_new_subdata_item ((mrss_item_t *) data, element, new);
    default:
      return MRSS_ERR_DATA;
    }
}

static mrss_error_t
__mrss_new_subdata_channel (mrss_t * mrss, mrss_element_t element,
			    mrss_generic_t data)
{
  mrss_item_t **item;
  mrss_hour_t **hour;
  mrss_day_t **day;
  mrss_category_t **category;
  int allocated;

  switch (element)
    {
    case MRSS_ELEMENT_ITEM:
      item = (mrss_item_t **) data;

      if (!*item)
	{
	  if (!(*item = (mrss_item_t *) malloc (sizeof (mrss_item_t))))
	    return MRSS_ERR_POSIX;

	  allocated = 1;
	}
      else
	allocated = 0;

      memset (*item, 0, sizeof (mrss_item_t));

      (*item)->element = MRSS_ELEMENT_ITEM;
      (*item)->allocated = allocated;
      (*item)->next = mrss->item;
      mrss->item = (*item);

      break;

    case MRSS_ELEMENT_SKIPHOURS:
      hour = (mrss_hour_t **) data;

      if (!*hour)
	{
	  if (!(*hour = (mrss_hour_t *) malloc (sizeof (mrss_hour_t))))
	    return MRSS_ERR_POSIX;

	  allocated = 1;
	}
      else
	allocated = 0;

      memset (*hour, 0, sizeof (mrss_hour_t));

      (*hour)->element = MRSS_ELEMENT_SKIPHOURS;
      (*hour)->allocated = allocated;
      (*hour)->next = mrss->skipHours;
      mrss->skipHours = (*hour);

      break;

    case MRSS_ELEMENT_SKIPDAYS:
      day = (mrss_day_t **) data;

      if (!*day)
	{
	  if (!(*day = (mrss_day_t *) malloc (sizeof (mrss_day_t))))
	    return MRSS_ERR_POSIX;

	  allocated = 1;
	}
      else
	allocated = 0;

      memset (*day, 0, sizeof (mrss_day_t));

      (*day)->element = MRSS_ELEMENT_SKIPDAYS;
      (*day)->allocated = allocated;
      (*day)->next = mrss->skipDays;
      mrss->skipDays = (*day);

      break;

    case MRSS_ELEMENT_CATEGORY:
      category = (mrss_category_t **) data;

      if (!*category)
	{
	  if (!
	      (*category =
	       (mrss_category_t *) malloc (sizeof (mrss_category_t))))
	    return MRSS_ERR_POSIX;
	  allocated = 1;
	}
      else
	allocated = 0;

      memset (*category, 0, sizeof (mrss_category_t));

      (*category)->element = MRSS_ELEMENT_CATEGORY;
      (*category)->allocated = allocated;
      (*category)->next = mrss->category;
      mrss->category = (*category);

      break;

    default:
      return MRSS_ERR_DATA;
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_new_subdata_item (mrss_item_t * item, mrss_element_t element,
			 mrss_generic_t data)
{
  mrss_category_t **category;
  int allocated;

  switch (element)
    {
    case MRSS_ELEMENT_CATEGORY:
      category = (mrss_category_t **) data;

      if (!*category)
	{
	  if (!
	      (*category =
	       (mrss_category_t *) malloc (sizeof (mrss_category_t))))
	    return MRSS_ERR_POSIX;

	  allocated = 1;
	}
      else
	allocated = 0;

      memset (*category, 0, sizeof (mrss_category_t));

      (*category)->element = MRSS_ELEMENT_CATEGORY;
      (*category)->allocated = allocated;
      (*category)->next = item->category;
      item->category = (*category);

      break;

    default:
      return MRSS_ERR_DATA;
    }

  return MRSS_OK;
}

mrss_error_t
mrss_remove_subdata (mrss_generic_t data, mrss_generic_t subdata)
{
  mrss_t *tmp;

  if (!data || !subdata)
    return MRSS_ERR_DATA;

  tmp = (mrss_t *) data;

  switch (tmp->element)
    {
    case MRSS_ELEMENT_CHANNEL:
      return __mrss_remove_subdata_channel ((mrss_t *) data, subdata);
    case MRSS_ELEMENT_ITEM:
      return __mrss_remove_subdata_item ((mrss_item_t *) data, subdata);
    default:
      return MRSS_ERR_DATA;
    }
}

static mrss_error_t
__mrss_remove_subdata_channel (mrss_t * data, mrss_generic_t subdata)
{
  mrss_hour_t *hour, *hour_tmp, *hour_old;
  mrss_day_t *day, *day_tmp, *day_old;
  mrss_category_t *category, *category_tmp, *category_old;
  mrss_item_t *item, *item_tmp, *item_old;

  int found = 0;

  mrss_t *tmp = (mrss_t *) subdata;

  switch (tmp->element)
    {
    case MRSS_ELEMENT_ITEM:
      item = (mrss_item_t *) subdata;

      item_tmp = data->item;
      item_old = NULL;

      while (item_tmp)
	{
	  if (item_tmp == item)
	    {
	      found++;

	      if (item_old)
		item_old->next = item_tmp->next;
	      else
		data->item = item_tmp->next;

	      break;
	    }

	  item_old = item_tmp;
	  item_tmp = item_tmp->next;
	}

      if (!found)
	return MRSS_ERR_DATA;

      break;

    case MRSS_ELEMENT_SKIPHOURS:
      hour = (mrss_hour_t *) subdata;

      hour_tmp = data->skipHours;
      hour_old = NULL;

      while (hour_tmp)
	{
	  if (hour_tmp == hour)
	    {
	      found++;

	      if (hour_old)
		hour_old->next = hour_tmp->next;
	      else
		data->skipHours = hour_tmp->next;

	      break;
	    }

	  hour_old = hour_tmp;
	  hour_tmp = hour_tmp->next;
	}

      if (!found)
	return MRSS_ERR_DATA;

      break;

    case MRSS_ELEMENT_SKIPDAYS:
      day = (mrss_day_t *) subdata;

      day_tmp = data->skipDays;
      day_old = NULL;

      while (day_tmp)
	{
	  if (day_tmp == day)
	    {
	      found++;

	      if (day_old)
		day_old->next = day_tmp->next;
	      else
		data->skipDays = day_tmp->next;

	      break;
	    }

	  day_old = day_tmp;
	  day_tmp = day_tmp->next;
	}

      if (!found)
	return MRSS_ERR_DATA;

      break;

    case MRSS_ELEMENT_CATEGORY:
      category = (mrss_category_t *) subdata;

      category_tmp = data->category;
      category_old = NULL;

      while (category_tmp)
	{
	  if (category_tmp == category)
	    {
	      found++;

	      if (category_old)
		category_old->next = category_tmp->next;
	      else
		data->category = category_tmp->next;

	      break;
	    }

	  category_old = category_tmp;
	  category_tmp = category_tmp->next;
	}

      if (!found)
	return MRSS_ERR_DATA;

      break;

    default:
      return MRSS_ERR_DATA;
    }

  return MRSS_OK;
}

static mrss_error_t
__mrss_remove_subdata_item (mrss_item_t * data, mrss_generic_t subdata)
{
  mrss_category_t *category, *category_tmp, *category_old;

  int found = 0;

  mrss_t *tmp = (mrss_t *) subdata;

  switch (tmp->element)
    {
    case MRSS_ELEMENT_CATEGORY:
      category = (mrss_category_t *) subdata;

      category_tmp = data->category;
      category_old = NULL;

      while (category_tmp)
	{
	  if (category_tmp == category)
	    {
	      found++;

	      if (category_old)
		category_old->next = category_tmp->next;
	      else
		data->category = category_tmp->next;

	      break;
	    }

	  category_old = category_tmp;
	  category_tmp = category_tmp->next;
	}

      if (!found)
	return MRSS_ERR_DATA;

      break;

    default:
      return MRSS_ERR_DATA;
    }

  return MRSS_OK;
}

/* EOF */
