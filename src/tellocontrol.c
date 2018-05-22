#include "tellocontrol.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gesture_recognition.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *box;
	Evas_Object *btn_connect;
	Evas_Object *btn_takeoff;

	int sock;
	struct sockaddr_in addr;

	gesture_h handle;

	bool is_connected;
	bool is_flying;
} appdata_s;

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
btn_connect_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;

	if(ad->is_connected){
		close(ad->sock);

		ad->is_connected = false;
		elm_object_text_set(ad->btn_connect, "Connect");
	}else{
		ad->sock = socket(AF_INET, SOCK_DGRAM, 0);
		ad->addr.sin_family = AF_INET;
		ad->addr.sin_port = htons(8889);
		ad->addr.sin_addr.s_addr = inet_addr("192.168.10.1");
		sendto(ad->sock, "command", 7, 0, (struct sockaddr *)&ad->addr, sizeof(ad->addr));

		ad->is_connected = true;
		elm_object_text_set(ad->btn_connect, "Disconnect");
	}
	evas_object_show(ad->btn_connect);
}

static void
btn_takeoff_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;

	if(ad->is_flying){
		sendto(ad->sock, "land", 4, 0, (struct sockaddr *)&ad->addr, sizeof(ad->addr));

		ad->is_flying = false;
		elm_object_text_set(ad->btn_takeoff, "Take off");
	}else{
		sendto(ad->sock, "takeoff", 7, 0, (struct sockaddr *)&ad->addr, sizeof(ad->addr));

		ad->is_flying = true;
		elm_object_text_set(ad->btn_takeoff, "Land");
	}
	evas_object_show(ad->btn_takeoff);
}

Eina_Bool
_rotary_handler_cb(void *data, Eext_Rotary_Event_Info *ev)
{
	appdata_s *ad = data;

	if (ad->is_flying){
		if (ev->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE) {
			sendto(ad->sock, "cw 15", 5, 0, (struct sockaddr *)&ad->addr, sizeof(ad->addr));
		} else {
			sendto(ad->sock, "ccw 15", 6, 0, (struct sockaddr *)&ad->addr, sizeof(ad->addr));
		}
		/*
		 The actual Gear S2 bezel moves by 15 degrees.
		 But rotating Tello by 15 degrees for each event actually feels slow.
		 You can increase the value by changing the parameter of sendto function.
		 */
	}
    return EINA_FALSE;
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Box */
	ad->box = elm_box_add(ad->conform);
	evas_object_size_hint_weight_set(ad->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->box);
	elm_object_content_set(ad->conform, ad->box);

	/* Create connect button */
	ad->btn_connect = elm_button_add(ad->conform);
	evas_object_size_hint_align_set(ad->btn_connect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/* Set the button text */
	elm_object_text_set(ad->btn_connect, "Connect");
	evas_object_show(ad->btn_connect);
	evas_object_smart_callback_add(ad->btn_connect, "clicked",
									btn_connect_clicked_cb, ad);
	elm_box_pack_end(ad->box, ad->btn_connect);

	/* Create takeoff button */
	ad->btn_takeoff = elm_button_add(ad->conform);
	evas_object_size_hint_align_set(ad->btn_takeoff, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/* Set the button text */
	elm_object_text_set(ad->btn_takeoff, "Take off");
	evas_object_show(ad->btn_takeoff);
	evas_object_smart_callback_add(ad->btn_takeoff, "clicked",
									btn_takeoff_clicked_cb, ad);
	elm_box_pack_end(ad->box, ad->btn_takeoff);

	/* Register Bezel event handler */
	eext_rotary_event_handler_add(_rotary_handler_cb, ad);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	ad->is_connected = false;
	ad->is_flying = false;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
