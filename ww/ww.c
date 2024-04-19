#include "ww.h"
#include "buffer_pool.h"
#include "hv/hloop.h"
#include "loggers/core_logger.h"
#include "loggers/dns_logger.h"
#include "loggers/network_logger.h"

#include "managers/node_manager.h"
#include "managers/socket_manager.h"

unsigned int             workers_count  = 0;
hthread_t               *workers        = NULL;
struct hloop_s         **loops          = NULL;
struct buffer_pool_s   **buffer_pools   = NULL;
struct socket_manager_s *socekt_manager = NULL;
struct node_manager_s   *node_manager   = NULL;
logger_t                *core_logger    = NULL;
logger_t                *network_logger = NULL;
logger_t                *dns_logger     = NULL;

struct ww_runtime_state_s
{
    unsigned int             workers_count;
    hthread_t               *workers;
    struct hloop_s         **loops;
    struct buffer_pool_s   **buffer_pools;
    struct socket_manager_s *socekt_manager;
    struct node_manager_s   *node_manager;
    logger_t                *core_logger;
    logger_t                *network_logger;
    logger_t                *dns_logger;
};

void setWW(struct ww_runtime_state_s *state)
{

    workers_count  = state->workers_count;
    workers        = state->workers;
    loops          = state->loops;
    buffer_pools   = state->buffer_pools;
    socekt_manager = state->socekt_manager;
    node_manager   = state->node_manager;
    setCoreLogger(state->core_logger);
    setNetworkLogger(state->network_logger);
    setDnsLogger(state->dns_logger);
    setSocketManager(socekt_manager);
    setNodeManager(node_manager);
    free(state);
}

struct ww_runtime_state_s *getWW()
{
    struct ww_runtime_state_s *state = malloc(sizeof(struct ww_runtime_state_s));
    memset(state, 0, sizeof(struct ww_runtime_state_s));
    state->workers_count  = workers_count;
    state->workers        = workers;
    state->loops          = loops;
    state->buffer_pools   = buffer_pools;
    state->socekt_manager = socekt_manager;
    state->node_manager   = node_manager;
    state->core_logger    = core_logger;
    state->network_logger = network_logger;
    state->dns_logger     = dns_logger;
    return state;
}

_Noreturn void runMainThread()
{
    hloop_run(loops[0]);
    hloop_free(&loops[0]);
    for (size_t i = 1; i < workers_count; i++)
    {
        hthread_join(workers[i]);
    }
    LOGF("WW: all eventloops exited");
    exit(0);
}

static HTHREAD_ROUTINE(worker_thread) //NOLINT
{
    hloop_t *loop = (hloop_t *) userdata;
    hloop_run(loop);
    hloop_free(&loop);

    return 0;
}

void createWW(ww_construction_data_t runtime_data)
{
    if (runtime_data.core_logger_data.log_file_path)
    {
        core_logger =
            createCoreLogger(runtime_data.core_logger_data.log_file_path, runtime_data.core_logger_data.log_console);
        setCoreLoggerLevelByStr(runtime_data.core_logger_data.log_level);
    }
    if (runtime_data.network_logger_data.log_file_path)
    {
        network_logger = createNetworkLogger(runtime_data.network_logger_data.log_file_path,
                                             runtime_data.network_logger_data.log_console);
        setNetworkLoggerLevelByStr(runtime_data.network_logger_data.log_level);
    }
    if (runtime_data.dns_logger_data.log_file_path)
    {
        dns_logger =
            createDnsLogger(runtime_data.dns_logger_data.log_file_path, runtime_data.dns_logger_data.log_console);
        setDnsLoggerLevelByStr(runtime_data.dns_logger_data.log_level);
    }

    workers_count = workers_count;
    workers       = (hthread_t *) malloc(sizeof(hthread_t) * workers_count);

    loops = (hloop_t **) malloc(sizeof(hloop_t *) * workers_count);
    for (int i = 1; i < workers_count; ++i)
    {
        loops[i]   = hloop_new(HLOOP_FLAG_AUTO_FREE);
        workers[i] = hthread_create(worker_thread, loops[i]);
    }
    loops[0]   = hloop_new(HLOOP_FLAG_AUTO_FREE);
    workers[0] = 0x0;

    buffer_pools = (struct buffer_pool_s **) malloc(sizeof(struct buffer_pool_s *) * workers_count);

    for (int i = 0; i < workers_count; ++i)
    {
        buffer_pools[i] = createBufferPool();
    }

    socekt_manager = createSocketManager();
    node_manager   = createNodeManager();
}