#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define BUILD_NAME_STR STRINGIFY(BUILD_NAME)
#define BUILD_DATE_STR STRINGIFY(__DATE__) " " STRINGIFY(__TIME__)
#define BUILD_USER_STR STRINGIFY(BUILD_USER)
#define BUILD_EMAIL_STR STRINGIFY(BUILD_EMAIL)

#define MAJOR_VERSION_STR STRINGIFY(MAJOR_VERSION)
#define MINOR_VERSION_STR STRINGIFY(MINOR_VERSION)
#define PATCH_VERSION_STR STRINGIFY(PATCH_VERSION)

#define GIT_COMMIT_STR STRINGIFY(GIT_COMMIT)
#define GIT_COMMIT_ABBREV_STR STRINGIFY(GIT_COMMIT_ABBREV)
#define GIT_BRANCH_STR STRINGIFY(GIT_BRANCH)
#if GIT_DIRTY > 0
#define GIT_DIRTY_STR "dirty"
#define GIT_DIRTY_APPEND_STR "_dirty"
#else
#define GIT_DIRTY_STR "clean"
#define GIT_DIRTY_APPEND_STR ""
#endif

#define GCC_VERSION_STR STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__) "." STRINGIFY(__GNUC_PATCHLEVEL__)

#if defined __OPTIMIZE__
#if defined __OPTIMIZE_SIZE
#define GCC_OPTIMISE_STR "size"
#else
#define GCC_OPTIMISE_STR "speed"
#endif
#else
#define GCC_OPTIMISE_STR "none"
#endif

/* Version string */
#define BUILD_VERSION_STR MAJOR_VERSION_STR "." MINOR_VERSION_STR "." PATCH_VERSION_STR

/* Pretty build ID (name, version) */
#define BUILD_DESC_STR BUILD_NAME_STR " v" BUILD_VERSION_STR


/* Git commit ID, branch name, dirty status */
#define GIT_REVISION_STR GIT_COMMIT_ABBREV_STR "_" GIT_BRANCH_STR GIT_DIRTY_APPEND_STR

/* Build name, revision info */
#define BUILD_ID_STR BUILD_NAME_STR "_" GIT_REVISION_STR

#define LOG_BUFFER 100
#define LINE_SIZE 200

/*** FreeRTOS helper stuff ***/

// Stack and TCB are placed in CCM of STM32F4
// The CCM block is connected directly to the core, which leads to zero wait states
//
// Macro to use CCM (Core Coupled Memory) in STM32F4
#define CCM_RAM __attribute__((section(".ccmram")))
