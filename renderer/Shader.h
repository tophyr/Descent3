/*
* Descent 3
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdexcept>
#include <string_view>
#include "dyna_gl.h"
#include "holder.h"

template <GLenum kType>
struct Shader {
  static_assert(kType == GL_VERTEX_SHADER || kType == GL_FRAGMENT_SHADER);

  explicit Shader(std::string_view src)
    : id_{dglCreateShader(kType)} {
    if (id_ == 0) {
      throw std::runtime_error("failed to create shader"); // TODO: better error checking?
    }

    char const* srcptr = src.data();
    GLint srclen = src.size();
    dglShaderSource(id_, 1, &srcptr, &srclen);
    dglCompileShader(id_);

    GLint compile_result;
    dglGetShaderiv(id_, GL_COMPILE_STATUS, &compile_result);
    if (compile_result != GL_TRUE) {
      GLsizei log_length = 0;
      GLchar message[1024];
      dglGetShaderInfoLog(id_, 1024, &log_length, message);
      throw std::runtime_error(std::string{message, static_cast<size_t>(log_length)});
    }
  }

  GLuint id() const {
    ASSERT(id_ != 0);
    return id_;
  }

private:
  static void deleteShader(GLuint id) {
    dglDeleteShader(id);
  }
  MoveOnlyHolder<GLuint, &deleteShader> id_;
};

using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;