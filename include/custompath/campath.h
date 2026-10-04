// ============================================================================
// Custom Camp Path - public API
// ============================================================================
#ifndef CUSTOMPATH_CAMPATH_H
#define CUSTOMPATH_CAMPATH_H

#include <cstddef>

namespace campath {

// Install the path hook. Returns false if the game base address is unknown or
// hook installation failed. Call once from HTModOnInit.
bool init() noexcept;

// Copy the resolved custom camp path (UTF-8) into `out`, NUL-terminated.
// Resolves lazily if init() has not run yet. No-op if out is null or size is 0.
void getCustomPath(char* out, std::size_t size) noexcept;

// Remove the hook and release COM. Called from DllMain on DLL_PROCESS_DETACH.
void shutdown() noexcept;

} // namespace campath

#endif // CUSTOMPATH_CAMPATH_H
