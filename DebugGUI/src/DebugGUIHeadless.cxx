#include <ostream>

#include "imgui.h"

namespace o2::framework {

// fills a stream with drawing data in JSON format
// the JSON is composed of a list of draw commands
/// FIXME: document the actual schema of the format.
void getFrameJSON(void *data, std::ostream &json_data) {
  ImDrawData *draw_data = (ImDrawData *)data;

  json_data << "[";

  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];
    const auto vtx_buffer = cmd_list->VtxBuffer;
    const auto idx_buffer = cmd_list->IdxBuffer;
    const auto cmd_buffer = cmd_list->CmdBuffer;

    json_data << "{\"vtx\":[";
    for (int i = 0; i < vtx_buffer.size(); ++i) {
      auto v = vtx_buffer[i];
      json_data << "[" << v.pos.x << "," << v.pos.y << "," << v.col << ','
                << v.uv.x << "," << v.uv.y << "]";
      if (i < vtx_buffer.size() - 1)
        json_data << ",";
    }

    json_data << "],\"idx\":[";
    for (int i = 0; i < idx_buffer.size(); ++i) {
      auto id = idx_buffer[i];
      json_data << id;
      if (i < idx_buffer.size() - 1)
        json_data << ",";
    }

    json_data << "],\"cmd\":[";
    for (int i = 0; i < cmd_buffer.size(); ++i) {
      auto cmd = cmd_buffer[i];
      json_data << "{\"cnt\":" << cmd.ElemCount << ", \"clp\":["
                << cmd.ClipRect.x << "," << cmd.ClipRect.y << ","
                << cmd.ClipRect.z << "," << cmd.ClipRect.w << "]}";
      if (i < cmd_buffer.size() - 1)
        json_data << ",";
    }
    json_data << "]}";
    if (cmd_id < draw_data->CmdListsCount - 1)
      json_data << ",";
  }

  json_data << "]";
}

struct vtxContainer {
  float posX, posY, uvX, uvY;
  int col;
};

struct cmdContainer {
  int count;
  float rectX, rectY, rectZ, rectW;
};

// given draw data, returns a mallocd buffer containing formatted frame data
// ready to be sent and its size.
// the returned buffer must be freed by the caller.
/// FIXME: document actual schema of the format
void getFrameRaw(void *data, void **raw_data, int *size) {
  ImDrawData *draw_data = (ImDrawData *)data;

  // compute sizes
  int buffer_size = sizeof(int) * 3, vtx_count = 0, idx_count = 0,
      cmd_count = 0;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    vtx_count += cmd_list->VtxBuffer.size();
    buffer_size += cmd_list->VtxBuffer.size() * (sizeof(vtxContainer));

    idx_count += cmd_list->IdxBuffer.size();
    buffer_size += cmd_list->IdxBuffer.size() * (sizeof(short));

    cmd_count += cmd_list->CmdBuffer.size();
    buffer_size += cmd_list->CmdBuffer.size() * (sizeof(cmdContainer));
  }

  void *local_data_base = malloc(buffer_size);

  int *data_header_ptr = (int *)local_data_base;
  *data_header_ptr = vtx_count;
  data_header_ptr++;
  *data_header_ptr = idx_count;
  data_header_ptr++;
  *data_header_ptr = cmd_count;
  data_header_ptr++;

  vtxContainer *data_vtx_ptr = (vtxContainer *)data_header_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const &vtx : cmd_list->VtxBuffer) {
      data_vtx_ptr->posX = vtx.pos.x;
      data_vtx_ptr->posY = vtx.pos.y;
      data_vtx_ptr->uvX = vtx.uv.x;
      data_vtx_ptr->uvY = vtx.uv.y;
      data_vtx_ptr->col = vtx.col;
      data_vtx_ptr++;
    }
  }

  int idx_offset = 0;
  short *data_idx_ptr = (short *)data_vtx_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const &idx : cmd_list->IdxBuffer) {
      *data_idx_ptr = idx + (short)idx_offset;
      data_idx_ptr++;
    }

    idx_offset += cmd_list->VtxBuffer.size();
  }

  cmdContainer *data_cmd_ptr = (cmdContainer *)data_idx_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const &cmd : cmd_list->CmdBuffer) {
      data_cmd_ptr->count = cmd.ElemCount;
      data_cmd_ptr->rectX = cmd.ClipRect.x;
      data_cmd_ptr->rectY = cmd.ClipRect.y;
      data_cmd_ptr->rectZ = cmd.ClipRect.z;
      data_cmd_ptr->rectW = cmd.ClipRect.w;
      data_cmd_ptr++;
    }
  }

  *size = buffer_size;
  *raw_data = local_data_base;
}
} // namespace o2::framework
