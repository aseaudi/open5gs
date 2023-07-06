#include "ogs-app.h"
#include "mme-context.h"

#include "metrics.h"

typedef struct mme_metrics_spec_def_s {
    unsigned int type;
    const char *name;
    const char *description;
    int initial_val;
    unsigned int num_labels;
    const char **labels;
} mme_metrics_spec_def_t;

/* Helper generic functions: */
static int mme_metrics_init_inst(ogs_metrics_inst_t **inst, ogs_metrics_spec_t **specs,
        unsigned int len, unsigned int num_labels, const char **labels)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        inst[i] = ogs_metrics_inst_new(specs[i], num_labels, labels);
    return OGS_OK;
}

static int mme_metrics_free_inst(ogs_metrics_inst_t **inst,
        unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        ogs_metrics_inst_free(inst[i]);
    memset(inst, 0, sizeof(inst[0]) * len);
    return OGS_OK;
}

static int mme_metrics_init_spec(ogs_metrics_context_t *ctx,
        ogs_metrics_spec_t **dst, mme_metrics_spec_def_t *src, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        dst[i] = ogs_metrics_spec_new(ctx, src[i].type,
                src[i].name, src[i].description,
                src[i].initial_val, src[i].num_labels, src[i].labels,
                NULL);
    }
    return OGS_OK;
}

/* GLOBAL */
ogs_metrics_spec_t *mme_metrics_spec_global[_MME_METR_GLOB_MAX];
ogs_metrics_inst_t *mme_metrics_inst_global[_MME_METR_GLOB_MAX];
mme_metrics_spec_def_t mme_metrics_spec_def_global[_MME_METR_GLOB_MAX] = {
/* Global Gauges: */
[MME_METR_GLOB_GAUGE_ENB_UE] = {
    .type = OGS_METRICS_METRIC_TYPE_GAUGE,
    .name = "enb_ue",
    .description = "Number of UEs connected to eNodeBs",
},
[MME_METR_GLOB_GAUGE_MME_SESS] = {
    .type = OGS_METRICS_METRIC_TYPE_GAUGE,
    .name = "mme_session",
    .description = "MME Sessions",
},
};

int mme_metrics_init_inst_global(void)
{
    return mme_metrics_init_inst(mme_metrics_inst_global, mme_metrics_spec_global,
                _MME_METR_GLOB_MAX, 0, NULL);
}
int mme_metrics_free_inst_global(void)
{
    return mme_metrics_free_inst(mme_metrics_inst_global, _MME_METR_GLOB_MAX);
}

/* LOCAL */
const char *labels_enb[] = {
    "connected"
};

ogs_metrics_inst_t *mme_metrics_inst_local = NULL;
ogs_metrics_spec_t *mme_metrics_spec_local[_MME_METR_LOCAL_MAX];
mme_metrics_spec_def_t mme_metrics_spec_def_local[_MME_METR_LOCAL_MAX] = {
/* Gauges: */
[MME_METR_LOCAL_GAUGE_ENB] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "enb",
        .description = "Status and IP address of eNBs that have connected to this MME",
        .num_labels = OGS_ARRAY_SIZE(labels_enb),
        .labels = labels_enb,
},
};

void mme_metrics_connected_enb_inc(char *ip_address)
{
    if ((NULL != mme_metrics_inst_local) &&
        (NULL != ip_address)) {
        // Increment the base counter (AKA "total")
        ogs_metrics_inst_inc(mme_metrics_inst_local);

        // Set connected IP to 1
        ogs_metrics_inst_set_with_label(mme_metrics_inst_local, ip_address, 1);
    } else {
        ogs_error("Failed to change eNB metrics as instance or ip address doesn't exist");
    }
}

void mme_metrics_connected_enb_dec(char *ip_address)
{
    if ((NULL != mme_metrics_inst_local) &&
        (NULL != ip_address)) {
        // Decrement the base counter (AKA "total")
        ogs_metrics_inst_dec(mme_metrics_inst_local);

        // Set connected IP to 0
        ogs_metrics_inst_set_with_label(mme_metrics_inst_local, ip_address, 0);
    } else {
        ogs_error("Failed to change eNB metrics as instance or ip address doesn't exist");
    }
}

int mme_metrics_init_inst_local(void)
{
    /* To get around a quirk of the prometheus lib we
     * pass in the key we want as first gauge key/val
     * pair instead of passing in the lables which seems
     * to be what its expecting. */
    const char *total_gauge_key[] = { "total" };
    mme_metrics_inst_local = ogs_metrics_inst_new(mme_metrics_spec_local[MME_METR_LOCAL_GAUGE_ENB], 1, total_gauge_key);

    return OGS_OK;
}

int mme_metrics_free_inst_local(void)
{
    return mme_metrics_free_inst(&mme_metrics_inst_local, _MME_METR_LOCAL_MAX);
}

void mme_metrics_init(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();
    ogs_metrics_context_init();

    mme_metrics_init_spec(ctx, mme_metrics_spec_global, mme_metrics_spec_def_global,
            _MME_METR_GLOB_MAX);

    mme_metrics_init_spec(ctx, mme_metrics_spec_local,
            mme_metrics_spec_def_local, _MME_METR_LOCAL_MAX);

    mme_metrics_init_inst_global();
    mme_metrics_init_inst_local();
}

void mme_metrics_final(void)
{
    ogs_metrics_context_final();
}
