#include <windows.h>
#include "includes/htmodloader.h"
#include "custompath/campath.h"

extern "C" __declspec(dllexport) HTStatus HTMLAPI HTModOnInit(void*) {
  if (!campath::init()) {
    HTTellText("§cCustomCampath: path hook init failed");
    return HT_FAIL;
  }

  char path[512];
  campath::getCustomPath(path, sizeof(path));
  // HTTellText("§aCustomCampath ready. Path: %s", path);
  return HT_SUCCESS;
}

extern "C" __declspec(dllexport) HTStatus HTMLAPI HTModOnEnable(void*) {
  return HT_SUCCESS;
}
