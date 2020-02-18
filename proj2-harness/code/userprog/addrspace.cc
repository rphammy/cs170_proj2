// addrspace.cc 
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option 
//  2. run coff2noff to convert the object file to Nachos format
//    (Nachos object code format is essentially just a simpler
//    version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//    (if you haven't implemented the file system yet, you
//    don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "machine.h" // definition of PageSize
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the 
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
  noffH->noffMagic = WordToHost(noffH->noffMagic);
  noffH->code.size = WordToHost(noffH->code.size);
  noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
  noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
  noffH->initData.size = WordToHost(noffH->initData.size);
  noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
  noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
  noffH->uninitData.size = WordToHost(noffH->uninitData.size);
  noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
  noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  First, set up the translation from program memory to physical 
//  memory.  For now, this is really simple (1:1), since we are
//  only uniprogramming, and we have a single unsegmented page table
//
//  "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable, PCB* newpcb)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
    (WordToHost(noffH.noffMagic) == NOFFMAGIC))
      SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
      + UserStackSize;  // we need to increase the size
            // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);   // check we're not trying
            // to run anything too big --
            // at least until we have
            // virtual memory

    memoryManager->lock->Acquire();
    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
          numPages, size);

    if (numPages <= memoryManager->getNumFreePages()) {

        this->pcb = newpcb;

        // Set up the page table
        pageTable = new TranslationEntry[numPages];
        for (i = 0; i < numPages; i++) {
            pageTable[i].virtualPage = i;
            pageTable[i].physicalPage = memoryManager->getPage();
            pageTable[i].valid = TRUE;
            pageTable[i].use = FALSE;
            pageTable[i].dirty = FALSE;
            pageTable[i].readOnly = FALSE;
        }
        memoryManager->lock->Release();
        
        machineLock->Acquire();
        // Zero out the physical pages allocated to this process.
        for (i = 0; i < numPages; i++) {
            int physAddr = pageTable[i].physicalPage * PageSize;
            bzero(&(machine->mainMemory[physAddr]), PageSize);
        }
        machineLock->Release();

        fprintf(stderr, "Loaded Program: %d code | %d data | %d bss\n",
            noffH.code.size, noffH.initData.size, noffH.uninitData.size);

        // then, copy in the code and data segments into memory using new
        // ReadFile functionality
        if (noffH.code.size > 0) {
            DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
                noffH.code.virtualAddr, noffH.code.size);
            ASSERT(ReadFile(noffH.code.virtualAddr,executable,noffH.code.size,
                noffH.code.inFileAddr) == 0);
        }
        if (noffH.initData.size > 0) {
            DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
                noffH.initData.virtualAddr, noffH.initData.size);
            ASSERT(ReadFile(noffH.initData.virtualAddr,executable,
                noffH.initData.size,noffH.initData.inFileAddr) == 0);
        }
    }

    else { // Not enough free pages to acquire.
        memoryManager->lock->Release();
        pageTable = NULL;
        newpcb = new PCB(-1,-1);
    }
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//     Copy constructor that makes an identical copy of this address space.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(const AddrSpace* other, PCB* newpcb) {
    
    ASSERT(other->numPages <= NumPhysPages);

    // Copy all page table entries over, create associated PCB
    numPages = other->numPages;
    memoryManager->lock->Acquire();
    DEBUG('a', "Initializing address space with num pages: %d.\n", numPages);

    if (numPages <= memoryManager->getNumFreePages()) {

        this->pcb = newpcb;
        pageTable = new TranslationEntry[numPages];

        //BEGIN HINTS
        //Set up the page table for the new process and allocate physical pages for each page in the new process under newpcb
        //ENDO HINTS
        //More hint: Look at sample code of AddrSpace::AddrSpace(OpenFile *executable, PCB* newpcb))
        //code  to see how it  sets up each page table entry of a new process. Need to fill every field of the page table entry.
        //See machine/translate.h for the definition of each  page table entry.
        //See memoryManager.cc on how to allocate memory pages.
        //Also (other->pageTable)[i] gives the i-th logical page table entry of "other" process.
        //Thus you will fill code inside the loop body of  for (int i = 0; i < numPages; i++) {  }


        memoryManager->lock->Release();

        machineLock->Acquire();
        //BEGIN HINTS
        //Copy page content of the other process to the new address space page by page to complete process content duplication
        //END HINTS
        //More hint: You will fill code inside the loop body of  for (int i = 0; i < numPages; i++) {  }
        //To get the physical addres (by byte) of the other process,
        //you can use (other->pageTable)[i].physicalPage * PageSize.
        //Then you can use bzero() to clean up and  use , bcopy() to complete the copy.

        machineLock->Release();
    }
    else {
        memoryManager->lock->Release();
        pageTable = NULL;
        pcb = new PCB(-1,-1);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//  Deallocate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    if (isValid()) {
        memoryManager->lock->Acquire();
        for (unsigned int i = 0; i < numPages; i++) { // free the pages
            memoryManager->clearPage(pageTable[i].physicalPage);
        }
        memoryManager->lock->Release();
        delete [] pageTable;
        delete pcb;
    }
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    machineLock->Acquire();
    for (i = 0; i < NumTotalRegs; i++)
  machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0); 

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    machineLock->Release();
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//----------------------------------------------------------------------
// AddrSpace::Translate
//
//     Converts a virtual address to a physical address.
//
//     Returns -1 for an error in translation, or else the corresponding
//     physical page number.
//----------------------------------------------------------------------

int AddrSpace::Translate(int virtualAddress) {

    int pageTableIndex = virtualAddress / PageSize;
    int offset = virtualAddress % PageSize;
    int physicalAddress = 0;

    if (virtualAddress < 0 || pageTableIndex > (int)numPages) {
        physicalAddress = -1;
    } else {
        physicalAddress = 
            pageTable[pageTableIndex].physicalPage * PageSize + offset;
    }

    return physicalAddress;
}

//----------------------------------------------------------------------
// AddrSpace::ReadFile
//     
//     Loads the code and data segments into the translated memory.
//----------------------------------------------------------------------

int AddrSpace::ReadFile(int virtAddr, OpenFile* file, int size, int fileAddr) {

    int numBytesRead = 0;
    int numBytesLeft = size;
    int numBytesToReadThisLoop = 0;
    int physAddr = 0;
    int currVirtAddr = virtAddr;
    int bytesFound = 0;

    while (numBytesRead < size) {

        if (numBytesLeft > PageSize) {
            numBytesToReadThisLoop = PageSize;
        } else {
            numBytesToReadThisLoop = numBytesLeft;
        }
        diskBufferLock->Acquire();
        bytesFound = file->ReadAt(diskBuffer, numBytesToReadThisLoop, fileAddr);
        diskBufferLock->Release();

        physAddr = Translate(currVirtAddr);

        if (physAddr != -1) { // zero the dest bytes, then copy over
            
            machineLock->Acquire();
            bzero(&(machine->mainMemory)[physAddr], numBytesToReadThisLoop);
            bcopy(&(diskBuffer[0]), &((machine->mainMemory)[physAddr]),
                numBytesToReadThisLoop);
            machineLock->Release();

            currVirtAddr += numBytesToReadThisLoop;
            numBytesLeft -= numBytesToReadThisLoop;

            numBytesRead += bytesFound;
            fileAddr += bytesFound;
        } 
        else {
            return -1;
        }
    }
    return 0;
}

//----------------------------------------------------------------------
// AddrSpace::getPCB
//     Returns the associated PCB for this address space.
//----------------------------------------------------------------------

PCB* AddrSpace::getPCB() {
    return this->pcb;
}

//----------------------------------------------------------------------
// AddrSpace::isValid
//     Checks that we were able to allocate a new address space successfully.
//----------------------------------------------------------------------

bool AddrSpace::isValid() {
    return (pcb != NULL);
}
