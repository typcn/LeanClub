cmake_minimum_required(VERSION 2.8)
project (hoedown)

set(SOURCE_FILES
    autolink.h
    autolink.c
    buffer.h
    buffer.c
    document.h
    document.c
    escape.h
    escape.c
    html_blocks.c
    html_smartypants.c
    html.c
    html.h
    stack.c
    stack.h
    version.c
    version.h
    )


add_library(hoedown ${SOURCE_FILES})