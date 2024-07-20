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

struct Program {
  explicit Program(VertexShader vertex, FragmentShader fragment)
    : vertex_{std::move(vertex)}, fragment_{std::move(fragment)}, id_{dglCreateProgram()} {
    if (id_ == 0) {
      throw std::runtime_error("error creating GL program");
    }

    dglAttachShader(id_, vertex_.id());
    dglAttachShader(id_, fragment_.id());

    dglLinkProgram(id_);

    GLint link_result;
    dglGetProgramiv(id_, GL_LINK_STATUS, &link_result);
    if (link_result != GL_TRUE)
    {
      GLsizei log_length = 0;
      GLchar message[1024];
      dglGetProgramInfoLog(id_, 1024, &log_length, message);
      throw std::runtime_error(std::string{message, static_cast<size_t>(log_length)});
    }

    dglUseProgram(id_); // TODO

    auto u_transform = dglGetUniformLocation(id_, "u_transform");
    if (u_transform == -1) {
      throw std::runtime_error("unable to find u_transform");
    }
    std::array<GLfloat, 4*4> identity {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    dglUniformMatrix4fv(u_transform, 1, GL_FALSE, identity.data());
  }

private:
  static void deleteProgram(GLuint id) {
    dglDeleteProgram(id);
  }

  VertexShader vertex_;
  FragmentShader fragment_;
  MoveOnlyHolder<GLuint, &deleteProgram> id_;
};