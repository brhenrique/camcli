cmake_minimum_required(VERSION 3.17)
project(cam_cli)

set(CMAKE_CXX_STANDARD 17)

add_executable(cam_cli media_source_stream.cpp media_source_stream.h utils.h camera.cpp camera.h window_renderer.cpp window_renderer.h color_converter.cpp color_converter.h stream_pipeline.cpp stream_pipeline.h mf_session.cpp mf_session.h main.cpp video_processor.cpp video_processor.h sample_stream.h sample_sink.h sample_filter.h win_graphics.h resizer.cpp resizer.h d3d_factory.cpp d3d_factory.h bitmap_sink.cpp bitmap_sink.h sample.h win_graphics.cpp)