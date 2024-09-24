cmake_minimum_required(VERSION 3.16)

if(DEFINED ESP_PLATFORM)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
endif()

project(doom)

if(NOT DEFINED ESP_PLATFORM)
add_subdirectory(../../ ${CMAKE_BINARY_DIR}/mcugdx)
set(SRC_DOOM
    main/doomgeneric/dummy.c main/doomgeneric/am_map.c main/doomgeneric/doomdef.c main/doomgeneric/doomstat.c main/doomgeneric/dstrings.c main/doomgeneric/d_event.c main/doomgeneric/d_items.c main/doomgeneric/d_iwad.c
    main/doomgeneric/d_loop.c main/doomgeneric/d_main.c main/doomgeneric/d_mode.c main/doomgeneric/d_net.c main/doomgeneric/f_finale.c main/doomgeneric/f_wipe.c main/doomgeneric/g_game.c main/doomgeneric/hu_lib.c
    main/doomgeneric/hu_stuff.c main/doomgeneric/info.c main/doomgeneric/i_cdmus.c main/doomgeneric/i_endoom.c main/doomgeneric/i_joystick.c main/doomgeneric/i_scale.c main/doomgeneric/i_sound.c
    main/doomgeneric/i_system.c main/doomgeneric/i_timer.c main/doomgeneric/memio.c main/doomgeneric/m_argv.c main/doomgeneric/m_bbox.c main/doomgeneric/m_cheat.c main/doomgeneric/m_config.c
    main/doomgeneric/m_controls.c main/doomgeneric/m_fixed.c main/doomgeneric/m_menu.c main/doomgeneric/m_misc.c main/doomgeneric/m_random.c main/doomgeneric/p_ceilng.c main/doomgeneric/p_doors.c
    main/doomgeneric/p_enemy.c main/doomgeneric/p_floor.c main/doomgeneric/p_inter.c main/doomgeneric/p_lights.c main/doomgeneric/p_map.c main/doomgeneric/p_maputl.c main/doomgeneric/p_mobj.c
    main/doomgeneric/p_plats.c main/doomgeneric/p_pspr.c main/doomgeneric/p_saveg.c main/doomgeneric/p_setup.c main/doomgeneric/p_sight.c main/doomgeneric/p_spec.c main/doomgeneric/p_switch.c
    main/doomgeneric/p_telept.c main/doomgeneric/p_tick.c main/doomgeneric/p_user.c main/doomgeneric/r_bsp.c main/doomgeneric/r_data.c main/doomgeneric/r_draw.c main/doomgeneric/r_main.c main/doomgeneric/r_plane.c
    main/doomgeneric/r_segs.c main/doomgeneric/r_sky.c main/doomgeneric/r_things.c main/doomgeneric/sha1.c main/doomgeneric/sounds.c main/doomgeneric/statdump.c main/doomgeneric/st_lib.c main/doomgeneric/st_stuff.c
    main/doomgeneric/s_sound.c main/doomgeneric/tables.c main/doomgeneric/v_video.c main/doomgeneric/wi_stuff.c main/doomgeneric/w_checksum.c main/doomgeneric/w_file.c main/doomgeneric/w_main.c
    main/doomgeneric/w_wad.c main/doomgeneric/z_zone.c main/doomgeneric/w_file_stdc.c main/doomgeneric/i_input.c main/doomgeneric/i_video.c main/doomgeneric/doomgeneric.c main/doomgeneric/mus2mid.c
)
add_executable(display "main/main.c" "${SRC_DOOM}")
target_link_libraries(display PUBLIC mcugdx)
mcugdx_create_rofs_partition(rofs "${CMAKE_CURRENT_SOURCE_DIR}/data/")
endif()