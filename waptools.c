/*
    Author:     ripmeep
    Instagram:  @rip.meep
    GitHub:     https://github.com/ripmeep
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <iwlib.h>

#include "ruby.h"
#include "extconf.h"


VALUE waptools_mod;
VALUE waptools_c_struct_WirelessAP;
VALUE waptools_c_struct_WirelessScanner;

typedef struct WirelessAPStruct
{
	char *ssid;
	char *bssid;
	char *key_type;
	char *freq;
	char *bitrate;
	VALUE rb_bitrate_hash;
	char *stats;
	int channel;
} WirelessAPStruct;

static const rb_data_type_t WirelessAPStruct_rb_type = {
	.wrap_struct_name = "WirelessAccessPoint",
	.function = {
		.dfree = RUBY_DEFAULT_FREE,
//		.dfree = WirelessAPStruct_free,
		.dsize = (const void*)sizeof(WirelessAPStruct),
	},
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

typedef struct WirelessScannerStruct
{
	char *interface;	// INTERFACE NAME
	int sockets; 		// WIRELESS SOCKETS

	wireless_scan *result;
	iwrange range;
	wireless_scan_head head;
} WirelessScannerStruct;

static const rb_data_type_t WirelessScanner_rb_type = {
	.wrap_struct_name = "WirelessScanner",
	.function = {
		.dfree = RUBY_DEFAULT_FREE,
//		.dfree = WirelessScannerStruct_free,
		.dsize = (const void*)sizeof(WirelessScannerStruct),
	},
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
};


void WirelessScannerStruct_free(WirelessScannerStruct *self);
void WirelessAPStruct_free(WirelessAPStruct *self);

static VALUE waptools_authors(VALUE self)
{
	return rb_str_new_cstr("Welcome to WAPTools! This module was made by ripmeep\nInstagram: @rip.meep\nGitHub:    https://github.com/ripmeep/");
}


static VALUE rb_WirelessAPStruct_alloc(VALUE klass)
{
	/* ALLOCATE APPROPRIATE STRUCT MEMORY FOR RB CLASS */
	return Data_Wrap_Struct(klass, NULL, WirelessAPStruct_free, ruby_xmalloc(sizeof(WirelessAPStruct)));
}

static VALUE rb_WirelessAPStruct_init(VALUE self)
{
	return self;
};


static VALUE rb_WirelessAPStruct_ssid(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (ap->ssid != NULL)
	{
		return rb_str_new_cstr(ap->ssid);
	}

	return Qnil;
}


static VALUE rb_WirelessAPStruct_bssid(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (ap->bssid != NULL)
	{
		return rb_str_new_cstr(ap->bssid);
	}

	return Qnil;
}

static VALUE rb_WirelessAPStruct_key_type(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (ap->key_type != NULL)
	{
		return rb_str_new_cstr(ap->key_type);
	}

	return Qnil;
}

static VALUE rb_WirelessAPStruct_freq(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (ap->freq != NULL)
	{
		return rb_str_new_cstr(ap->freq);
	}

	return Qnil;
}

static VALUE rb_WirelessAPStruct_bitrate(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (RB_TYPE_P(ap->rb_bitrate_hash, T_HASH))
	{
		return ap->rb_bitrate_hash;
	} else if (ap->bitrate != NULL)
	{
		return rb_str_new_cstr(ap->bitrate);
	}

	return Qnil;
}

static VALUE rb_WirelessAPStruct_stats(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	if (ap->stats != NULL)
	{
		return rb_str_new_cstr(ap->stats);
	}

	return Qnil;
}

static VALUE rb_WirelessAPStruct_channel(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	return INT2NUM(ap->channel);
}

static VALUE rb_WirelessAPStruct_to_s(VALUE self)
{
	WirelessAPStruct *ap;
	TypedData_Get_Struct(self, WirelessAPStruct, &WirelessAPStruct_rb_type, ap);

	size_t info_len = 2048;

	if (ap->ssid == NULL || ap->bssid == NULL)
		return Qnil;

	return rb_sprintf("SSID:      %s\nBSSID:     %s\nFREQUENCY: %s\nBITRATE:   %s\nSTATS:     %s\nCHANNEL:   %d",
		ap->ssid, ap->bssid, ap->freq, ap->bitrate, ap->stats, ap->channel
	);
}


static VALUE rb_WirelessScannerStruct_alloc(VALUE klass)
{
	/* ALLOCATE APPROPRIATE STRUCT MEMORY FOR RB CLASS */
	return Data_Wrap_Struct(klass, NULL, WirelessScannerStruct_free, ruby_xmalloc(sizeof(WirelessScannerStruct)));
}

static VALUE rb_WirelessScannerStruct_init(VALUE self, VALUE iface_name)
{
	WirelessScannerStruct *scanner;

	Check_Type(iface_name, T_STRING); /* ENSURE "iface_name" IS OF STRING TYPE */

	Data_Get_Struct(self, WirelessScannerStruct, scanner); /* DESERIALIZE DATA FROM CLASS TO STRUCTURE */

	scanner->sockets = iw_sockets_open();

	if (scanner->sockets < 0)
	{
		rb_raise(rb_eRuntimeError, "Unable to open wireless sockets [%s]", strerror(errno)); /* RAISE EXCEPTION AND RETURN NIL IF ERROR OPENING SOCKETS */
		return Qnil;
	}

	char *c_iface_name = StringValuePtr(iface_name);

	if (iw_get_range_info(scanner->sockets, c_iface_name, &scanner->range) < 0)
	{
		rb_raise(rb_eRuntimeError, "Could not get range info for interface \"%s\" [%s]", c_iface_name, strerror(errno)); /* RAISE EXCEPTION AND RETURN NIL IF UNABLE TO GET INTERFACE RANGE */
		return Qnil; /* WILL ALSO RETURN NIL IF DEVICE DOESN'T EXIST */
	}

	scanner->interface = (char*)malloc(RSTRING_LEN(iface_name) + 1);
	memcpy(scanner->interface, c_iface_name, RSTRING_LEN(iface_name) + 1); /* COPY "iface_name" TO INTERFACE */

	return self; /* RETURN NEW CLASS */
}

static VALUE rb_WirelessScannerStruct_interface(VALUE self)
{
	WirelessScannerStruct *s;
	Data_Get_Struct(self, WirelessScannerStruct, s); /* DESERIALIZE DATA FROM CLASS TO STRUCTURE */

	if (s->interface != NULL)
	{
		return rb_str_new_cstr(s->interface); /* RETURN INTERFACE IF SET */
	}

	return Qnil;
}

static VALUE rb_WirelessScannerStruct_scan(VALUE self)
{
	WirelessScannerStruct *s;
	Data_Get_Struct(self, WirelessScannerStruct, s); /* DESERIALIZE DATA FROM CLASS TO STRUCTURE */

	if (iw_scan(s->sockets, s->interface, s->range.we_version_compiled, &s->head) < 0) /* SCAN */
	{
		rb_raise(rb_eRuntimeError, "Failed while scanning on interface \"%s\" [%s]", s->interface, strerror(errno)); /* RAISE EXCEPTION AND RETURN NIL IF SCAN FAILS */
		return Qnil;
	}

	s->result = s->head.result;

	VALUE rb_access_points = rb_ary_new();
	VALUE rb_ap;

	long xap = 0;
	char nuller[256];

	WirelessAPStruct *ap = (WirelessAPStruct*)malloc(sizeof(WirelessAPStruct) + 1);

	while (s->result != NULL)
	{

		ap->ssid = (char*)malloc(strlen(s->result->b.essid) + 1);

		if (ap->ssid != NULL)
		{
			memcpy(ap->ssid, s->result->b.essid, strlen(s->result->b.essid)); /* STORE SSID */
			ap->ssid[strlen(s->result->b.essid)] = '\0';
		}

		if (s->result->has_ap_addr)
		{
			iw_sawap_ntop(&s->result->ap_addr, nuller); /* CONVERT BSSID TO READABLE FORMAT */
			ap->bssid = (char*)malloc(strlen(nuller) + 1);

			if (ap->bssid != NULL)
			{
				memcpy(ap->bssid, nuller, strlen(nuller));
				ap->bssid[strlen(nuller)] = '\0';
			}

			memset(nuller, '\0', sizeof(nuller)); /* ZERO OUT BUFFER HANDLE */
		}

		if (s->result->b.has_freq)
		{
			iw_print_freq_value(nuller, sizeof(nuller), s->result->b.freq); /* PRINT FREQUENCY */
			ap->freq = (char*)malloc(strlen(nuller) + 1);

			if (ap->freq != NULL)
			{
				memcpy(ap->freq, nuller, strlen(nuller));
				ap->freq[strlen(nuller)] = '\0';

				double dfreq;

				char *t = strtok(strdup(ap->freq), " ");

				if (t != NULL)
				{
					sscanf(t, "%lf", &dfreq);
					ap->channel = iw_freq_to_channel(dfreq * 1000000000, &s->range);
				} else
				{
					ap->channel = 0;
				}

			} else
			{
				ap->channel = 0;
			}

			memset(nuller, '\0', sizeof(nuller)); /* ZERO OUT BUFFER HANDLE */
		}

		if (s->result->has_maxbitrate)
		{
			iw_print_bitrate(nuller, sizeof(nuller), s->result->maxbitrate.value); /* PRINT BITRATE */
			ap->bitrate = (char*)malloc(strlen(nuller) + 1);

			if (ap->bitrate != NULL)
			{
				memcpy(ap->bitrate, nuller, strlen(nuller));
				ap->bitrate[strlen(nuller)] = '\0';
				ap->rb_bitrate_hash = rb_hash_new();

				char *t = strtok(strdup(ap->bitrate), " ");

				if (t != NULL)
				{
					int i = 0;

					while (t != NULL)
					{
						if (i == 0)
						{
							int rate = strtoul(t, NULL, 10);

							if (rate > 0)
							{
								rb_hash_aset(ap->rb_bitrate_hash, rb_str_new_cstr("value"), INT2NUM(rate));
							}
						} else if (i == 1)
						{
							rb_hash_aset(ap->rb_bitrate_hash, rb_str_new_cstr("rate"), rb_str_new_cstr(t));
						}

						i++;
						t = strtok(NULL, " ");
					}
				}
			}

			memset(nuller, '\0', sizeof(nuller)); /* ZERO OUT BUFFER HANDLE */
		}

		if (s->result->has_stats)
		{
			iw_print_stats(nuller, sizeof(nuller), &s->result->stats.qual, &s->range, 1); /* PRINT STATS */
			ap->stats = (char*)malloc(strlen(nuller) + 1);

			if (ap->stats != NULL)
			{
				memcpy(ap->stats, nuller, strlen(nuller));
				ap->stats[strlen(nuller)] = '\0';
				memset(nuller, '\0', sizeof(nuller)); /* ZERO OUT BUFFER HANDLE */
			}
		}

		if (strlen(ap->ssid) > 0 && strlen(ap->bssid) > 0) /* ONLY RETRIEVE NETWORKS WE CAN FULLY SEE */
		{
			rb_ap = TypedData_Make_Struct(waptools_c_struct_WirelessAP, WirelessAPStruct, &WirelessAPStruct_rb_type, ap); /* CREATE EXTERNAL ACCESS POINT CLASS FROM STRUCTURE */
			rb_ary_store(rb_access_points, xap, rb_ap); /* STORE ACCESS POINT IN ARRAY */
		}

		xap++;

		s->result = s->result->next;

		memset(nuller, '\0', sizeof(nuller)); /* ZERO OUT BUFFER HANDLE */
	}

	s->result = s->head.result;

	// FREE ALL RESULTS

	while (s->result && s->result->next)
	{
		wireless_scan *tmp = s->result->next;
		free(s->result);
		s->result = tmp;
	}

	if (s->result)
	{
		free(s->result);
	}

	return rb_access_points; /* RETURN ACCESS POINTS ARRAY */
}

void Init_waptools() /* RUBY LOAD MODULE */
{
	waptools_mod = rb_define_module("WAPTools"); /* INIT MODULE */
	waptools_c_struct_WirelessAP = rb_define_class_under(waptools_mod, "WirelessAccessPoint", rb_cObject);
	waptools_c_struct_WirelessScanner = rb_define_class_under(waptools_mod, "Scanner", rb_cObject); /* ADD WIRELESS SCANNER CLASS ONTO MODULE FROM C STRUCTURE */

	rb_define_singleton_method(waptools_mod, "authors", waptools_authors, 0);

	rb_define_alloc_func(waptools_c_struct_WirelessAP, rb_WirelessAPStruct_alloc);
	rb_define_method(waptools_c_struct_WirelessAP, "initialize", rb_WirelessAPStruct_init, 0); /* INTIIALIZER FOR WAP CLASS */
	rb_define_method(waptools_c_struct_WirelessAP, "ssid", rb_WirelessAPStruct_ssid, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "bssid", rb_WirelessAPStruct_bssid, 0);
//	rb_define_method(waptools_c_struct_WirelessAP, "key_type", rb_WirelessAPStruct_key_type, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "frequency", rb_WirelessAPStruct_freq, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "bitrate", rb_WirelessAPStruct_bitrate, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "stats", rb_WirelessAPStruct_stats, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "channel", rb_WirelessAPStruct_channel, 0);
	rb_define_method(waptools_c_struct_WirelessAP, "to_s", rb_WirelessAPStruct_to_s, 0);

	rb_define_alloc_func(waptools_c_struct_WirelessScanner, rb_WirelessScannerStruct_alloc);
	rb_define_method(waptools_c_struct_WirelessScanner, "initialize", rb_WirelessScannerStruct_init, 1); /* INTIIALIZER FOR SCANNER CLASS */
	rb_define_method(waptools_c_struct_WirelessScanner, "interface", rb_WirelessScannerStruct_interface, 0);
	rb_define_method(waptools_c_struct_WirelessScanner, "scan", rb_WirelessScannerStruct_scan, 0);
}

void WirelessAPStruct_free(WirelessAPStruct *self)
{
	if (self != NULL)
	{
		free(self);
	}
}

void WirelessScannerStruct_free(WirelessScannerStruct *self)
{
	if (self->sockets > 0)
	{
		iw_sockets_close(self->sockets);
	}

	if (self != NULL)
	{
		free(self);
	}
}

