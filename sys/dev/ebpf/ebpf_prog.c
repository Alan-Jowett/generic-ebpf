/*-
 * SPDX-License-Identifier: Apache License 2.0
 *
 * Copyright 2017-2018 Yutaro Hayakawa
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dev/ebpf/ebpf_prog.h>

const struct ebpf_prog_type *ebpf_prog_types[] = {
	[EBPF_PROG_TYPE_BAD]  = &bad_prog_type,
	[EBPF_PROG_TYPE_TEST] = &test_prog_type
};

const struct ebpf_prog_type *
ebpf_get_prog_type(uint16_t type)
{
	if (type >= EBPF_PROG_TYPE_MAX)
		return NULL;

	return ebpf_prog_types[type];
}

static void
ebpf_prog_dtor(struct ebpf_obj *eo)
{
	struct ebpf_prog *ep = (struct ebpf_prog *)eo;

	for (uint16_t i = 0; i < ep->ndep_maps; i++)
		ebpf_obj_release((struct ebpf_obj *)ep->dep_maps[i]);

	ebpf_free(ep->prog);
}

int
ebpf_prog_create(struct ebpf_prog **eopp, struct ebpf_prog_attr *attr)
{
	struct ebpf_prog *ep;

	if (eopp == NULL || attr == NULL ||
			attr->type >= EBPF_PROG_TYPE_MAX ||
			attr->prog == NULL || attr->prog_len == 0)
		return EINVAL;

	ep = ebpf_malloc(sizeof(*ep));
	if (ep == NULL)
		return ENOMEM;

	ep->prog = ebpf_malloc(attr->prog_len);
	if (ep->prog == NULL) {
		ebpf_free(ep);
		return ENOMEM;
	}

	memcpy(ep->prog, attr->prog, attr->prog_len);

	ebpf_refcount_init(&ep->eo.eo_ref, 1);
	ep->eo.eo_type = EBPF_OBJ_TYPE_PROG;
	ep->eo.eo_dtor = ebpf_prog_dtor;
	ep->type = attr->type;
	ep->ndep_maps = 0;
	ep->prog_len = attr->prog_len;

	memset(ep->dep_maps, 0,
			sizeof(ep->dep_maps[0]) * EOP_MAX_DEPS);

	*eopp = ep;

	return 0;
}

void
ebpf_prog_destroy(struct ebpf_prog *ep)
{
	ebpf_obj_release(&ep->eo);
}

int
ebpf_prog_attach_map(struct ebpf_prog *ep, struct ebpf_map *em)
{
	if (ep == NULL || em == NULL)
		return EINVAL;

	if (ep->ndep_maps >= EOP_MAX_DEPS)
		return EBUSY;

	ebpf_obj_acquire((struct ebpf_obj *)em);
	ep->dep_maps[ep->ndep_maps++] = em;

	return 0;
}
