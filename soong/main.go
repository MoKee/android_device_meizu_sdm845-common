package sdm845

import (
    "android/soong/android"
)

func init() {
    android.RegisterModuleType("meizu_sdm845_fod_hal_binary", fodHalBinaryFactory)
}
