/*
 * Descent 3
 * Copyright (C) 2026 Descent Developers
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

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace pof {

struct Vec {
  float x = 0, y = 0, z = 0;
};

struct FaceVert {
  int vertnum;
  float u, v;
};

struct Face {
  Vec normal;
  int texnum = -1; // -1 = untextured, use rgb
  uint8_t r = 0, g = 0, b = 0;
  std::vector<FaceVert> verts;
};

struct SubObject {
  int parent = -1;
  Vec offset;
  std::string name;
  std::string props;
  std::vector<Vec> verts;
  std::vector<float> alpha; // version >= 2300 only; defaults to 1.0
  std::vector<Face> faces;
};

struct Point {
  int parent = 0;
  Vec pnt;
  Vec norm;
};

struct RotKey {
  int start_time = 0; // timed (version >= 2200) models only
  Vec axis;
  int angle = 0; // 16-bit fixed angle, 0x4000 == 90 degrees
};

struct PosKey {
  int start_time = 0; // timed (version >= 2200) models only
  Vec pos;
};

struct Track {
  int track_min = 0, track_max = 0; // timed models only
  std::vector<RotKey> rot;
  std::vector<PosKey> pos;
};

struct WeaponBattery {
  std::vector<int> gun_points;
  std::vector<int> turrets;
};

class Writer {
public:
  explicit Writer(int version) : version_(version) {
    raw("PSPO");
    i32(version);
  }

  int version() const { return version_; }

  void header(int n_models, float rad = 0.0f, Vec mins = {}, Vec maxs = {}) {
    begin("OHDR");
    i32(n_models);
    f32(rad);
    vec(mins);
    vec(maxs);
    i32(0); // detail level count
    end();
  }

  void subobject(int index, const SubObject &so) {
    const int major = version_ / 100;
    begin("SOBJ");
    i32(index);
    i32(so.parent);
    vec({0, 0, 1}); // separation plane normal (unused by the new-style loader)
    f32(0);         // separation plane d
    vec({});        // separation plane point
    vec(so.offset);
    f32(0); // radius
    i32(0); // tree offset
    i32(0); // data offset
    if (version_ > 1805)
      vec({}); // geometric center
    str(so.name);
    str(so.props);
    i32(0); // movement type
    i32(0); // movement axis
    i32(0); // freespace chunk count
    i32(static_cast<int>(so.verts.size()));
    for (const Vec &v : so.verts)
      vec(v);
    for (size_t i = 0; i < so.verts.size(); i++)
      vec({0, 0, 1}); // vertex normals
    if (major >= 23) {
      for (size_t i = 0; i < so.verts.size(); i++)
        f32(i < so.alpha.size() ? so.alpha[i] : 1.0f);
    }
    i32(static_cast<int>(so.faces.size()));
    for (const Face &f : so.faces) {
      vec(f.normal);
      i32(static_cast<int>(f.verts.size()));
      if (f.texnum >= 0) {
        i32(1);
        i32(f.texnum);
      } else {
        i32(0);
        u8(f.r);
        u8(f.g);
        u8(f.b);
      }
      for (const FaceVert &fv : f.verts) {
        i32(fv.vertnum);
        f32(fv.u);
        f32(fv.v);
      }
      if (major >= 21) {
        f32(0); // lightmap x resolution
        f32(0); // lightmap y resolution
      }
    }
    end();
  }

  void textures(const std::vector<std::string> &names) {
    begin("TXTR");
    i32(static_cast<int>(names.size()));
    for (const std::string &n : names)
      str(n);
    end();
  }

  void gun_points(const std::vector<Point> &points) {
    begin("GPNT");
    i32(static_cast<int>(points.size()));
    for (const Point &p : points) {
      if (version_ >= 1908)
        i32(p.parent);
      vec(p.pnt);
      vec(p.norm);
    }
    end();
  }

  void ground_planes(const std::vector<Point> &points) {
    begin("GRND");
    i32(static_cast<int>(points.size()));
    for (const Point &p : points) {
      i32(p.parent);
      vec(p.pnt);
      vec(p.norm);
    }
    end();
  }

  void attach_points(const std::vector<Point> &points) {
    begin("ATCH");
    i32(static_cast<int>(points.size()));
    for (const Point &p : points) {
      i32(p.parent);
      vec(p.pnt);
      vec(p.norm);
    }
    end();
  }

  void attach_uvecs(const std::vector<Vec> &uvecs) {
    begin("NATH");
    i32(static_cast<int>(uvecs.size()));
    for (const Vec &u : uvecs) {
      i32(0);  // unused
      vec({}); // unused
      vec(u);
    }
    end();
  }

  void weapon_batteries(const std::vector<WeaponBattery> &wbs) {
    begin("WBAT");
    i32(static_cast<int>(wbs.size()));
    for (const WeaponBattery &wb : wbs) {
      i32(static_cast<int>(wb.gun_points.size()));
      for (int g : wb.gun_points)
        i32(g);
      i32(static_cast<int>(wb.turrets.size()));
      for (int t : wb.turrets)
        i32(t);
    }
    end();
  }

  // One Track per subobject, in subobject order. Untimed models (version < 2200) share one key
  // count across all subobjects, taken from tracks[0].
  void rot_anim(const std::vector<Track> &tracks) {
    const bool timed = version_ / 100 >= 22;
    begin("RANI");
    if (!timed)
      i32(static_cast<int>(tracks.at(0).rot.size()));
    for (const Track &t : tracks) {
      if (timed) {
        i32(static_cast<int>(t.rot.size()));
        i32(t.track_min);
        i32(t.track_max);
      }
      for (const RotKey &k : t.rot) {
        if (timed)
          i32(k.start_time);
        vec(k.axis);
        i32(k.angle);
      }
    }
    end();
  }

  void pos_anim(const std::vector<Track> &tracks) {
    const bool timed = version_ / 100 >= 22;
    begin("PANI");
    if (!timed)
      i32(static_cast<int>(tracks.at(0).pos.size()));
    for (const Track &t : tracks) {
      if (timed) {
        i32(static_cast<int>(t.pos.size()));
        i32(t.track_min);
        i32(t.track_max);
      }
      for (const PosKey &k : t.pos) {
        if (timed)
          i32(k.start_time);
        vec(k.pos);
      }
    }
    end();
  }

  void unknown_chunk(const char id[4], const std::vector<uint8_t> &payload) {
    begin(id);
    buf_.insert(buf_.end(), payload.begin(), payload.end());
    end();
  }

  void save(const std::filesystem::path &path) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(buf_.data()), static_cast<std::streamsize>(buf_.size()));
  }

private:
  void raw(const char id[4]) { buf_.insert(buf_.end(), id, id + 4); }
  void u8(uint8_t v) { buf_.push_back(v); }
  void i32(int32_t v) {
    for (int i = 0; i < 4; i++)
      buf_.push_back(static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xff));
  }
  void f32(float f) {
    int32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));
    i32(bits);
  }
  void vec(const Vec &v) {
    f32(v.x);
    f32(v.y);
    f32(v.z);
  }
  // The count includes a terminating NUL because the loader does not terminate the buffer itself.
  void str(const std::string &s) {
    i32(static_cast<int>(s.size() + 1));
    buf_.insert(buf_.end(), s.begin(), s.end());
    buf_.push_back(0);
  }

  void begin(const char id[4]) {
    raw(id);
    len_pos_ = buf_.size();
    i32(0);
  }
  void end() {
    const uint32_t len = static_cast<uint32_t>(buf_.size() - len_pos_ - 4);
    for (int i = 0; i < 4; i++)
      buf_[len_pos_ + i] = static_cast<uint8_t>((len >> (8 * i)) & 0xff);
  }

  int version_;
  std::vector<uint8_t> buf_;
  size_t len_pos_ = 0;
};

} // namespace pof
