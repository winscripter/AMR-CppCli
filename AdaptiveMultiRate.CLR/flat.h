// Copyright (c) 2023-2026, winscripter


#pragma once

#include "windows.h"
#include "narrowband/interf_dec.h"
#include "narrowband/interf_enc.h"
#include "wideband/dec_if.h"
#include "wideband/enc_if.h"


//
// Macro for a flat API export.
//
#define FLATAPI __declspec(dllexport) __stdcall
