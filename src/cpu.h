#ifndef CPU_H
#define CPU_H

#include "common.h"
#include "instructions.h"

typedef struct {
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    u16 sp;
    u16 pc;
} cpu_registers;

typedef struct {
    cpu_registers regs;

    // current fetch ... (when we grab a register?)
    u16 fetched_data; // data que acaba de ser leido en el fetch? (antes de decoe y exec)
    u16 mem_dest; // memory destination (may or not be set)
    bool dest_is_mem; // dest is memory or no
    u8 cur_opcode; // Permite identificar que tipo de instruccion se esta ejecutando, para que la CPU
    instruction *cur_inst; // current instruction

    bool halted; // Es un indicador que muestra si la CPU esta en estado halted (detenida)
    bool stepping; //Cuando esta activado, la CPU ejecutara solo una instruccion y luego se detendra (for debugging)

    bool int_master_enable;
    u8 ie_register; // interrupt enable register
} cpu_context;

void cpu_init();
bool cpu_step();

// function pointer: return void & takes a cpu_context pointer
typedef void (*IN_PROC)(cpu_context *); // es un tipo de dato

// we'll get the instruction processor by the instruction type
// so we can create a function for each instruction
IN_PROC inst_get_processor(in_type type);

// macros to get the flags
#define CPU_FLAG_Z BIT(ctx->regs.f, 7)
#define CPU_FLAG_C BIT(ctx->regs.f, 4)

u16 cpu_read_reg(reg_type rt);

void cpu_set_reg(reg_type rt, u16 val);

void fetch_data();

u8 cpu_get_ie_register();
void cpu_set_ie_register(u8 n);

#endif

/*

1. fetch_data:
Descripcion: Es el dato que acaba de ser recogido o "fetch" desde
la memoria o desde el bus de datos.
En otras palabras, cuando la CPU esta ejecutando un ciclo de instruccion,
primero debe leer (fetch) la instruccion desde la memoria.
Esta variable almacena el valor que se ha recuperado.

Funcion: Almacena la instruccion o datos que han sido leidos en ese momento,
antes de que sean decodificados o ejecutados.

2. mem_dest:
Descripcion: Es la "direccion de memoria de destino" a la que la CPU pretende acceder.
Este campo puede ser utilizado en operaciones de lectura o escritura,
y puede o no estar configurado, dependiendo de la operacion que este ejecutando la CPU.

Funcion: En una operacion de lectura o escritura, mem_dest indica a donde se debe leer
o escribir. Por ejemplo, si la instruccion implica cargar un valor en un registro
desde una direccion de memoria especifica, mem_dest tendra esa direccion.

*/