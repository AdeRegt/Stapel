#include <stdint.h>

#ifdef STAPELOS
#include "../../../kernel/include/stapelbridge.h"
#else
typedef struct {
  void* central_memory;
  void* stack;
  void* call_stack;
  uint64_t instruction_pointer;
  uint64_t stack_pointer;
  uint64_t call_stack_pointer;
} __attribute__((packed)) StapelMultitaskingInstance;
#endif

typedef struct {
    uint8_t signature[2];
    uint64_t version;
    uint8_t architecture;
}__attribute__((packed)) StapelFileHeader;

#define STAPEL_LIMITATION_STACK                     100

#define STAPEL_HEADER_SIGNATURE_A                   'S'
#define STAPEL_HEADER_SIGNATURE_B                   'T'
#define STAPEL_HEADER_VERSION                       2
#define STAPEL_HEADER_ARCHITECTURE                  2

#define STAPEL_INSTRUCTION_EXIT                     0x00
#define STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_64    0x01
#define STAPEL_INSTRUCTION_PUSH_VALUE_64            0x02
#define STAPEL_INSTRUCTION_DEBUG                    0x03
#define STAPEL_INSTRUCTION_ADD                      0x04
#define STAPEL_INSTRUCTION_SUB                      0x05
#define STAPEL_INSTRUCTION_MUL                      0x06
#define STAPEL_INSTRUCTION_DIV                      0x07
#define STAPEL_INSTRUCTION_CALL                     0x08
#define STAPEL_INSTRUCTION_JUMP                     0x09
#define STAPEL_INSTRUCTION_INT                      0x0A
#define STAPEL_INSTRUCTION_JUMP_EQUALS              0x0B
#define STAPEL_INSTRUCTION_JUMP_MORE                0x0C
#define STAPEL_INSTRUCTION_JUMP_LESS                0x0D
#define STAPEL_INSTRUCTION_RET                      0x0E
#define STAPEL_INSTRUCTION_POP_64                   0x0F
#define STAPEL_INSTRUCTION_PUSH_RAW_ADDR            0x10
#define STAPEL_INSTRUCTION_SYSCALL                  0x11
#define STAPEL_INSTRUCTION_PUSH_VALUE_8             0x12
#define STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_8     0x13
#define STAPEL_INSTRUCTION_POP_8                    0x14
#define STAPEL_INSTRUCTION_POP_AT_64                0x15
#define STAPEL_INSTRUCTION_POP_AT_8                 0x16

#define STAPEL_SYSCALL_VERSION                      1