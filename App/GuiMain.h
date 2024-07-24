#pragma once


using drawcallback = void(void*);

int GuiMain(drawcallback drawfunction, void* obj_ptr);
std::string reverse_utf8(const std::string_view input);