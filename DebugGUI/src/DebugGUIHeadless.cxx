#include <ostream>

#include "imgui.h"

namespace o2::framework {

struct __attribute__((packed)) FrameInfo {
  int vtx_count = 0;
  int idx_count = 0;
  int cmd_count = 0;
};

struct __attribute__((packed)) vtxContainer {
  float posX, posY, uvX, uvY;
  int col;
};

struct __attribute__((packed)) cmdContainer {
  int list_id;
  int cmd_id;
  int count;
  int vtxOffset;
  int idxOffset;
  int vtxCount;
  int idxCount;
  int vtxBase;
  int idxBase;
  float rectX, rectY, rectZ, rectW;
};

// given draw data, returns a mallocd buffer containing formatted frame data
// ready to be sent and its size.
// the returned buffer must be freed by the caller.
/// FIXME: document actual schema of the format
void getFrameRaw(void *data, void **raw_data, int *size) {
  auto *draw_data = (ImDrawData *)data;
  FrameInfo frameInfo;

  // compute sizes
  int buffer_size = sizeof(int) * 3;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    frameInfo.vtx_count += cmd_list->VtxBuffer.size();
    buffer_size += cmd_list->VtxBuffer.size() * (sizeof(vtxContainer));

    frameInfo.idx_count += cmd_list->IdxBuffer.size();
    buffer_size += cmd_list->IdxBuffer.size() * (sizeof(short));

    frameInfo.cmd_count += cmd_list->CmdBuffer.size();
    buffer_size += cmd_list->CmdBuffer.size() * (sizeof(cmdContainer));
  }

  void *local_data_base = (char *)malloc(buffer_size);
  char *ptr = (char *)local_data_base;

  memcpy(ptr, &frameInfo, sizeof(FrameInfo));
  ptr += sizeof(FrameInfo);

  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];
    memcpy(ptr, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
    ptr += cmd_list->VtxBuffer.size() * sizeof(ImDrawVert);
  }

  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];
    memcpy(ptr, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
    ptr += cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx);
  }

  int vtxBase = 0;
  int idxBase = 0;
  for (int list_id = 0; list_id < draw_data->CmdListsCount; ++list_id) {
    const auto cmd_list = draw_data->CmdLists[list_id];

    for (int cmd_id = 0; cmd_id < cmd_list->CmdBuffer.size(); ++cmd_id) {
      const auto cmd = cmd_list->CmdBuffer[cmd_id];
      cmdContainer op;
      op.list_id = list_id;
      op.cmd_id = cmd_id;
      op.count = cmd.ElemCount;
      op.vtxOffset = cmd.VtxOffset;
      op.idxOffset = cmd.IdxOffset;
      op.vtxCount = cmd_list->VtxBuffer.size();
      op.idxCount = cmd_list->IdxBuffer.size();
      op.vtxBase = vtxBase;
      op.idxBase = idxBase;
      op.rectX = cmd.ClipRect.x;
      op.rectY = cmd.ClipRect.y;
      op.rectZ = cmd.ClipRect.z;
      op.rectW = cmd.ClipRect.w;
      memcpy(ptr, &op, sizeof(cmdContainer));
      ptr += sizeof(cmdContainer);
    }
    vtxBase += cmd_list->VtxBuffer.size();
    idxBase += cmd_list->IdxBuffer.size();
  }

  *size = buffer_size;
  *raw_data = local_data_base;
}

} // namespace o2::framework
