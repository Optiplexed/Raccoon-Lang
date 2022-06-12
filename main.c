#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum
   {
   PUSH = 1,
   ADD,
   SUB,
   MUL,
   DIV,
   
   EQ,
   NEQ,
   GT,
   LT,
   GTE,
   LTE,
   
   AND,
   OR,
   XOR,
   
   NOT,
   NEGATE,
   AS_CHAR,
   
   MODULO,
   SHL,
   SHR,
   
   TEE,
   SET,
   GET
   } Opcode;

typedef struct
   {
   #define STACK_SIZE 32
   int stack[STACK_SIZE];
   size_t stackTop;
   
   #define INSTRUCTION_SIZE 256
   int instructions[INSTRUCTION_SIZE];
   size_t instrTop;
   
   #define MEMORY_SIZE 64
   int memory[MEMORY_SIZE];
   
   } VirtualMachine;
   
typedef enum
   {
   NEED_TWO_ARGS = 1,
   ILLEGAL_OP_CODE,
   NEED_ONE_ARG,
   MEMORY_OUT_OF_BOUNDS
   } VirtualMachineError;
   
char* VirtualMachineErrorMsg[] = {
   "Expected 2 Arguments for this operation",
   "Illegal Operator Code (Either negative or too large)",
   "Expected 1 argument for this operation"
   };
   
VirtualMachine* VirtualMachine_new()
   {
   VirtualMachine *vm = calloc(1, sizeof(VirtualMachine));
   return vm;
   }
   
void VirtualMachine_exit(VirtualMachine *vm, VirtualMachineError errorCode)
   {
   puts(VirtualMachineErrorMsg[errorCode - 1]);
   exit(errorCode);
   }
void VirtualMachine_pushInt(VirtualMachine *vm, int value)
   {
   vm->stack[vm->stackTop++] = value;
   }
   
void VirtualMachine_add(VirtualMachine *vm)
   {
   if(vm->stackTop >= 2)
      {
      vm->stack[vm->stackTop - 2] = vm->stack[vm->stackTop - 2] + vm->stack[vm->stackTop - 1];
      vm->stack[vm->stackTop - 1] = 0;
      vm->stackTop -= 1;
      }
   else
      {
      VirtualMachine_exit(vm, NEED_TWO_ARGS);
      }
   }
  
#define VirtualMachine_opcode2(name, op) \
void VirtualMachine_ ## name (VirtualMachine *vm) \
   { \
   if(vm->stackTop >= 2) \
      { \
      vm->stack[vm->stackTop - 2] = vm->stack[vm->stackTop - 2] op vm->stack[vm->stackTop - 1]; \
      vm->stack[vm->stackTop - 1] = 0; \
      vm->stackTop -= 1; \
      } \
   else \
      { \
      VirtualMachine_exit(vm, NEED_TWO_ARGS); \
      } \
   }
void VirtualMachine_set(VirtualMachine *vm)
   {
   if(vm->stackTop >= 2)
      {   
      int location = vm->stack[vm->stackTop - 2];
      int value = vm->stack[vm->stackTop - 1];
      
      if(location > MEMORY_SIZE || location < 0)
         {
         VirtualMachine_exit(vm, MEMORY_OUT_OF_BOUNDS);
         }
      vm->memory[location] = value;
      vm->stack[vm->stackTop - 1] = 0;
      vm->stack[vm->stackTop - 2] = 0;
      
      vm->stackTop -= 2;
      }
   else
      {
      VirtualMachine_exit(vm, NEED_TWO_ARGS);
      }
   }
   
void VirtualMachine_get(VirtualMachine *vm)
   {
   if(vm->stackTop >= 1)
      {   
      int location = vm->stack[vm->stackTop - 1];
      
      if(location > MEMORY_SIZE || location < 0)
         {
         VirtualMachine_exit(vm, MEMORY_OUT_OF_BOUNDS);
         }
      vm->stack[vm->stackTop - 1] = vm->memory[location];
      }
   else
      {
      VirtualMachine_exit(vm, NEED_ONE_ARG);
      }
   }
   
void VirtualMachine_tee(VirtualMachine *vm)
   {
   if(vm->stackTop >= 2)
      {   
      int location = vm->stack[vm->stackTop - 2];
      int value = vm->stack[vm->stackTop - 1];
      
      if(location > MEMORY_SIZE || location < 0)
         {
         VirtualMachine_exit(vm, MEMORY_OUT_OF_BOUNDS);
         }
      vm->memory[location] = value;
      vm->stack[vm->stackTop - 2] = 0;
      vm->stack[vm->stackTop - 1] = vm->memory[location];
      
      vm->stackTop--;
      }
   else
      {
      VirtualMachine_exit(vm, NEED_TWO_ARGS);
      }
   }
   
VirtualMachine_opcode2(sub, -)
VirtualMachine_opcode2(mul, *)
VirtualMachine_opcode2(div, /)

VirtualMachine_opcode2(eq, ==)
VirtualMachine_opcode2(neq, !=)
VirtualMachine_opcode2(gt, >)
VirtualMachine_opcode2(lt, <)
VirtualMachine_opcode2(gte, >=)
VirtualMachine_opcode2(lte, <=)

VirtualMachine_opcode2(and, &)
VirtualMachine_opcode2(or, |)
VirtualMachine_opcode2(xor, ^)

VirtualMachine_opcode2(mod, %)
VirtualMachine_opcode2(shl, <<)
VirtualMachine_opcode2(shr, >>)
   
void VirtualMachine_not(VirtualMachine *vm)
   {
   if(vm->stackTop >= 1)
      {
      vm->stack[vm->stackTop] = !(vm->stack[vm->stackTop]);
      }
   else
      {
      VirtualMachine_exit(vm, NEED_ONE_ARG);
      }
   }
   
void VirtualMachine_negate(VirtualMachine *vm)
   {
   if(vm->stackTop >= 1)
      {
      vm->stack[vm->stackTop] *= -1;
      }
   else
      {
      VirtualMachine_exit(vm, NEED_ONE_ARG);
      }
   }
   
void VirtualMachine_asChar(VirtualMachine *vm)
   {
   if(vm->stackTop >= 1)
      {
      printf("%c", vm->stack[vm->stackTop]);
      }
   else
      {
      VirtualMachine_exit(vm, NEED_ONE_ARG);
      }
   }

void VirtualMachine_addInstruction(VirtualMachine *vm, int opCode)
   {
   vm->instructions[vm->instrTop++] = opCode;
   }
void VirtualMachine_execute(VirtualMachine *vm)
   {
   size_t pointer = 0;
   bool pushMode = false;
   while(true)
      {
      if(pushMode)
         {
         int toPush = vm->instructions[pointer++];
         VirtualMachine_pushInt(vm, toPush);
         pushMode = false;
         }
      else
         {
         switch(vm->instructions[pointer++])
            {
            case PUSH:
            pushMode = true;
            break;
            
            #define instruction(op, func) \
            case op: \
            VirtualMachine_ ## func(vm); \
            break
            
            instruction(ADD, add);
            instruction(SUB, sub);
            instruction(MUL, mul);
            instruction(DIV, div);
            
            instruction(EQ, eq);
            instruction(NEQ, neq);
            instruction(GT, gt);
            instruction(LT, lt);
            instruction(GTE, gte);
            instruction(LTE, lte);
            
            instruction(AND, and);
            instruction(OR, or);
            instruction(XOR, xor);
            
            instruction(NOT, not);
            instruction(NEGATE, negate);
            instruction(AS_CHAR, asChar);
            
            instruction(MODULO, mod);
            instruction(SHL, shl);
            instruction(SHR, shr);
            
            instruction(TEE, tee);
            instruction(GET, get);
            instruction(SET, set);
         
            case 0:
            return; 
          
            default:
            VirtualMachine_exit(vm, ILLEGAL_OP_CODE);
            }
         }
      }
   }
void VirtualMachine_print(VirtualMachine *vm)
   {
   printf("stack pointer: %zu\n", vm->stackTop);
   for(size_t i = 0; i < STACK_SIZE; i++)
      {
      if(!((i + 1) % 5))
         {
         printf("%d", vm->stack[i]);
         printf("\n");
         }
      else
         {
         printf("%d\t", vm->stack[i]);
         }
      }
   }
   
int main()
   {
   VirtualMachine* vm = VirtualMachine_new();
   
   VirtualMachine_addInstruction(vm, PUSH);
   VirtualMachine_addInstruction(vm, 10);
   VirtualMachine_addInstruction(vm, PUSH);
   VirtualMachine_addInstruction(vm, 10);
   VirtualMachine_addInstruction(vm, ADD);
   VirtualMachine_addInstruction(vm, PUSH);
   VirtualMachine_addInstruction(vm, 2);
   VirtualMachine_addInstruction(vm, MUL);
   
   VirtualMachine_execute(vm);
   VirtualMachine_print(vm);
   }