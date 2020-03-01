/*
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <stdint.h>

struct mixer_ctl {
    void *mixer;
    void *info;
    char **ename;
    bool info_retrieved;
};

extern int mixer_ctl_get_valxx(struct mixer_ctl *ctl, unsigned int id);

extern int mixer_ctl_get_value(struct mixer_ctl *ctl, unsigned int id) {
    return mixer_ctl_get_valxx(ctl, id);
}
