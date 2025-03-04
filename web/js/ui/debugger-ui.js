class DebuggerUI {
	constructor() {
		this.debugger = window.nesDebugger;
		this.breakpoints = new Set();
		this.currentMemoryPage = 0x0000;
		this.memoryPageSize = 0x100;

		this.setupEventListeners();

		window.addEventListener('debugger-ready', () => {
			this.updateUI();
		});

		if (this.debugger && this.debugger.isLoaded) {
			this.updateUI();
		}
	}

	setupEventListeners() {
		document.getElementById('resetButton').addEventListener('click', () => this.reset());
		document.getElementById('runButton').addEventListener('click', () => this.run());
		document.getElementById('stepButton').addEventListener('click', () => this.step());
		document.getElementById('stopButton').addEventListener('click', () => this.stop());
		document.getElementById('addBreakpointButton').addEventListener('click', () => this.addBreakpoint());
		document.getElementById('jumpToAddressButton').addEventListener('click', () => this.jumpToMemoryAddress());
		document.getElementById('memoryPage').addEventListener('change', () => this.changeMemoryPage());
		document.getElementById('loadRomButton').addEventListener('click', () => {
			document.getElementById('romFile').click();
		});

		document.getElementById('romFile').addEventListener('change', (e) => {
			this.debugger.setPC(0x0200);
			this.loadROMFile(e.target.files[0]);
		});

		document.getElementById('loadOpcodesButton').addEventListener('click', () => {
			this.debugger.setPC(0x0200);
			this.loadOpcodesFromText();
		});

		window.addEventListener('nes-debugger-update', () => this.updateUI());
		document.getElementById('disassemblyView').addEventListener('click', (e) => {
			const row = e.target.closest('.instruction');
			if (row) {
				const addr = parseInt(row.dataset.address, 16);
				this.toggleBreakpoint(addr);
			}
		});

		document.getElementById('memoryView').addEventListener('click', (e) => {
			const cell = e.target.closest('.memory-cell');
			if (cell && cell.dataset.address) {
				this.promptEditMemoryValue(parseInt(cell.dataset.address, 16));
			}
		});

		this.initTooltips();
	}

	initTooltips() {
		const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
		[...tooltipTriggerList].forEach(tooltipTriggerEl => {
			new bootstrap.Tooltip(tooltipTriggerEl, {
				trigger: 'hover focus',
				dismiss: 'click',
				html: true
			});

			tooltipTriggerEl.addEventListener('click', () => {
				const tooltip = bootstrap.Tooltip.getInstance(tooltipTriggerEl);
				if (tooltip) {
					tooltip.hide();
				}
			});
		});
	}

	reset() {
		this.debugger.reset();
		this.updateUI();
		this.showToast('CPU reset successfully', 'success');
	}

	run() {
		this.debugger.startContinuousExecution();
		this.showToast('Execution started', 'info');
	}

	step() {
		this.debugger.stepInstruction();
	}

	stop() {
		this.debugger.stopContinuousExecution();
		this.updateUI();
		this.showToast('Execution stopped', 'warning');
	}

	addBreakpoint() {
		const input = document.getElementById('breakpointAddress');
		const addrStr = input.value.trim();

		if (addrStr) {
			try {
				const addr = parseInt(addrStr, 16);
				if (!isNaN(addr) && addr >= 0 && addr <= 0xFFFF) {
					this.debugger.addBreakpoint(addr);
					this.breakpoints.add(addr);
					input.value = '';
					this.updateBreakpointsList();
					this.updateDisassembly();
					this.showToast(`Breakpoint added at $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'success');
				} else {
					this.showToast('Invalid address range', 'danger');
				}
			} catch (e) {
				console.error('Invalid breakpoint address:', e);
				this.showToast('Invalid address format', 'danger');
			}
		}
	}

	removeBreakpoint(addr) {
		this.debugger.removeBreakpoint(addr);
		this.breakpoints.delete(addr);
		this.updateBreakpointsList();
		this.updateDisassembly();
		this.showToast(`Breakpoint removed from $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'info');
	}

	toggleBreakpoint(addr) {
		if (this.breakpoints.has(addr)) {
			this.removeBreakpoint(addr);
		} else {
			this.debugger.addBreakpoint(addr);
			this.breakpoints.add(addr);
			this.updateBreakpointsList();
			this.updateDisassembly();
			this.showToast(`Breakpoint toggled at $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'success');
		}
	}

	updateBreakpointsList() {
		const list = document.getElementById('breakpointsList');
		list.innerHTML = '';

		if (this.breakpoints.size === 0) {
			list.innerHTML = '<div class="text-muted small">No breakpoints set</div>';
			return;
		}

		const sortedBreakpoints = Array.from(this.breakpoints).sort((a, b) => a - b);

		for (const bp of sortedBreakpoints) {
			const item = document.createElement('div');
			item.className = 'd-flex justify-content-between align-items-center mb-2';

			const addrSpan = document.createElement('span');
			addrSpan.className = 'badge bg-light text-dark';
			addrSpan.textContent = `$${bp.toString(16).toUpperCase().padStart(4, '0')}`;

			const removeButton = document.createElement('button');
			removeButton.innerHTML = '<i class="bi bi-x-circle"></i>';
			removeButton.className = 'btn btn-sm btn-outline-danger';
			removeButton.setAttribute('data-bs-toggle', 'tooltip');
			removeButton.setAttribute('data-bs-placement', 'left');
			removeButton.setAttribute('title', 'Remove Breakpoint');
			removeButton.addEventListener('click', () => this.removeBreakpoint(bp));

			item.appendChild(addrSpan);
			item.appendChild(removeButton);
			list.appendChild(item);
		}

		// Re-initialize tooltips for the new buttons
		this.initTooltips();
	}

	jumpToMemoryAddress() {
		const input = document.getElementById('memoryAddress');
		const addrStr = input.value.trim();

		if (addrStr) {
			try {
				const addr = parseInt(addrStr, 16);
				if (!isNaN(addr) && addr >= 0 && addr <= 0xFFFF) {
					this.currentMemoryPage = addr & 0xFF00;
					this.updateMemoryView();
					this.showToast(`Jumped to memory address $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'info');
				} else {
					this.showToast('Invalid address range', 'danger');
				}
			} catch (e) {
				console.error('Invalid memory address:', e);
				this.showToast('Invalid address format', 'danger');
			}
		}
	}

	changeMemoryPage() {
		const select = document.getElementById('memoryPage');
		const value = select.value;

		switch (value) {
			case 'zeropage':
				this.currentMemoryPage = 0x0000;
				this.memoryPageSize = 0x100;
				break;
			case 'stack':
				this.currentMemoryPage = 0x0100;
				this.memoryPageSize = 0x100;
				break;
			case 'ram':
				this.currentMemoryPage = 0x0200;
				this.memoryPageSize = 0x100;
				break;
			case 'vectors':
				this.currentMemoryPage = 0xFFFA;
				this.memoryPageSize = 6; // Just show the 6 bytes of vectors
				break;
			default:
				this.currentMemoryPage = 0x0000;
				this.memoryPageSize = 0x100;
		}

		this.updateMemoryView();
		this.showToast(`Memory view changed to ${select.options[select.selectedIndex].text}`, 'info');
	}

	loadROMFile(file) {
		if (!file) return;

		const reader = new FileReader();
		reader.onload = (e) => {
			const data = new Uint8Array(e.target.result);
			this.debugger.loadROM(data);
			this.updateUI();
			this.showToast(`ROM loaded: ${file.name} (${data.length} bytes)`, 'success');
		};
		reader.readAsArrayBuffer(file);
	}

	loadOpcodesFromText() {
		const textArea = document.getElementById('opcodeInput');
		const opcodesText = textArea.value.trim();

		if (!opcodesText) {
			this.showToast('Please enter opcodes', 'warning');
			return;
		}

		try {
			const opcodes = this.parseOpcodes(opcodesText);

			if (opcodes.length === 0) {
				this.showToast('No valid opcodes found', 'warning');
				return;
			}

			const data = new Uint8Array(opcodes);
			this.debugger.loadROM(data);
			this.updateUI();
			this.showToast(`Loaded ${data.length} bytes successfully`, 'success');

		} catch (e) {
			console.error('Error loading opcodes:', e);
			this.showToast(`Error: ${e.message}`, 'danger');
		}
	}

	parseOpcodes(text) {
		text = text.replace(/;.*$/gm, '');

		const hexValues = text.replace(/[\s,]+/g, ' ').trim().split(' ');
		const opcodes = [];
		for (const hex of hexValues) {
			if (!hex) continue;

			if (!/^[0-9A-Fa-f]{1,2}$/.test(hex)) {
				throw new Error(`Invalid opcode format: ${hex}`);
			}

			const value = parseInt(hex, 16);
			opcodes.push(value);
		}

		return opcodes;
	}

	promptEditMemoryValue(address) {
		const currentValue = this.debugger.readMemory(address);
		const newValueStr = prompt(
			`Edit memory at $${address.toString(16).toUpperCase().padStart(4, '0')}\nCurrent value: $${currentValue.toString(16).toUpperCase().padStart(2, '0')}\nEnter new value (hex):`,
			currentValue.toString(16).toUpperCase().padStart(2, '0')
		);

		if (newValueStr === null) return; // User canceled

		try {
			const newValue = parseInt(newValueStr, 16);
			if (isNaN(newValue) || newValue < 0 || newValue > 255) {
				this.showToast('Invalid byte value (must be 00-FF)', 'danger');
				return;
			}

			this.debugger.writeMemory(address, newValue);
			this.updateMemoryView();
			this.showToast(`Memory at $${address.toString(16).toUpperCase().padStart(4, '0')} updated to $${newValue.toString(16).toUpperCase().padStart(2, '0')}`, 'success');
		} catch (e) {
			console.error('Error updating memory:', e);
			this.showToast('Error updating memory value', 'danger');
		}
	}

	showToast(message, type = 'info') {
		let toastContainer = document.querySelector('.toast-container');
		if (!toastContainer) {
			toastContainer = document.createElement('div');
			toastContainer.className = 'toast-container position-fixed bottom-0 end-0 p-3';
			document.body.appendChild(toastContainer);
		}

		const toastId = 'toast-' + Date.now();
		const toast = document.createElement('div');
		toast.className = `toast align-items-center text-white bg-${type} border-0`;
		toast.setAttribute('id', toastId);
		toast.setAttribute('role', 'alert');
		toast.setAttribute('aria-live', 'assertive');
		toast.setAttribute('aria-atomic', 'true');

		toast.innerHTML = `
			<div class="d-flex">
				<div class="toast-body">
					${message}
				</div>
				<button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast" aria-label="Close"></button>
			</div>
		`;

		toastContainer.appendChild(toast);
		// Initialize and show the toast
		const bsToast = new bootstrap.Toast(toast, {
			autohide: true,
			delay: 3000
		});
		bsToast.show();

		// Remove toast after it's hidden
		toast.addEventListener('hidden.bs.toast', () => {
			toast.remove();
		});
	}

	updateUI() {
		this.updateRegisters();
		this.updateFlags();
		this.updateDisassembly();
		this.updateMemoryView();
		this.updateStats();
	}

	updateRegisters() {
		const state = this.debugger.getState();
		if (!state) return;

		const regs = state.registers;

		document.getElementById('reg-a').textContent = `$${regs.A.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-x').textContent = `$${regs.X.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-y').textContent = `$${regs.Y.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-sp').textContent = `$${regs.SP.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-pc').textContent = `$${regs.PC.toString(16).toUpperCase().padStart(4, '0')}`;
	}

	updateFlags() {
		const state = this.debugger.getState();
		if (!state) return;

		const flags = state.registers.flags;

		document.getElementById('flag-n').className = flags.N ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-v').className = flags.V ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-u').className = flags.U ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-b').className = flags.B ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-d').className = flags.D ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-i').className = flags.I ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-z').className = flags.Z ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-c').className = flags.C ? 'badge bg-success' : 'badge bg-secondary';

		// Update text content to show 1/0
		document.getElementById('flag-n').textContent = flags.N ? '1' : '0';
		document.getElementById('flag-v').textContent = flags.V ? '1' : '0';
		document.getElementById('flag-u').textContent = flags.U ? '1' : '0';
		document.getElementById('flag-b').textContent = flags.B ? '1' : '0';
		document.getElementById('flag-d').textContent = flags.D ? '1' : '0';
		document.getElementById('flag-i').textContent = flags.I ? '1' : '0';
		document.getElementById('flag-z').textContent = flags.Z ? '1' : '0';
		document.getElementById('flag-c').textContent = flags.C ? '1' : '0';
	}

	parseIntSafe(str, defaultValue = 0) {
		if (str === undefined || str === null || str === '') {
			return defaultValue;
		}
		const parsed = parseInt(str, 10);
		return isNaN(parsed) ? defaultValue : parsed;
	}

	// Function to safely parse disassembly data from the C++ backend
	parseDisassemblyInstructions(rawString) {
		if (!rawString) {
			console.error('Empty disassembly data received');
			return [];
		}

		// Each instruction is separated by '#'
		const instructions = rawString.split('#');

		// Process each instruction
		const result = [];
		let nextAddr = null; // Track expected address for next instruction

		for (let i = 0; i < instructions.length; i++) {
			const instrStr = instructions[i].trim();
			if (!instrStr) continue;

			// Each instruction field is separated by '|'
			const parts = instrStr.split('|');

			// Need at least address, opcode, mnemonic
			if (parts.length < 3) {
				console.warn('Incomplete instruction data:', instrStr);
				continue;
			}

			// Try to get the expected fields
			const addr = parseIntSafe(parts[0]);
			const opcode = parseIntSafe(parts[1]);
			const mnemonic = parts[2] || 'UNK';
			const operand = parseIntSafe(parts[3], 0);
			const formatted = parts[4] || mnemonic;
			const bytes = parseIntSafe(parts[5], 1);
			const cycles = parseIntSafe(parts[6], 0);

			// If we got a weird address but we know what it should be, fix it
			const finalAddr = isNaN(addr) || addr <= 0 ? nextAddr : addr;

			// Create the instruction object with valid data
			const instr = {
				address: finalAddr,
				opcode: opcode,
				mnemonic: mnemonic,
				operand: operand,
				formatted: formatted,
				bytes: bytes,
				cycles: cycles
			};

			// Calculate next expected address
			if (finalAddr !== null && bytes > 0) {
				nextAddr = finalAddr + bytes;
			} else {
				nextAddr = null;
			}

			result.push(instr);
		}

		// Now go through and fix any remaining address issues
		for (let i = 0; i < result.length; i++) {
			if (isNaN(result[i].address) || result[i].address === null) {
				// If this isn't the first instruction, calculate from previous
				if (i > 0 && result[i - 1].address !== null && result[i - 1].bytes > 0) {
					result[i].address = result[i - 1].address + result[i - 1].bytes;
				} else {
					// Can't determine a valid address
					result[i].address = 0;
				}
			}
		}

		return result;
	}

	// Modify your existing disassembleAroundPC to use this function
	disassembleAroundPC(before, after) {
		const rawData = Module.ccall(
			'debugger_disassemble_around_pc',
			'string',
			['number', 'number'],
			[before, after]
		);

		return parseDisassemblyInstructions(rawData);
	}

	// And similarly for disassembleRange
	disassembleRange(start, end) {
		const rawData = Module.ccall(
			'debugger_disassemble_range',
			'string',
			['number', 'number'],
			[start, end]
		);

		return parseDisassemblyInstructions(rawData);
	}

	// And update the updateDisassembly method to better handle the instruction data
	updateDisassembly() {
		const view = document.getElementById('disassemblyView');
		view.innerHTML = '';

		try {
			// Get disassembly data
			const instructions = this.debugger.disassembleAroundPC(5, 30);
			const pc = this.debugger.getRegisterPC();

			// Display each instruction
			let lastAddr = null;
			for (let i = 0; i < instructions.length; i++) {
				const instr = instructions[i];
				// Skip instructions with invalid addresses or that seem to be parsing errors
				if (instr.address === 0 || instr.mnemonic === String(instr.opcode)) {
					console.warn('Skipping invalid instruction:', instr);
					continue;
				}

				// Check for address continuity - may indicate a parsing issue
				if (lastAddr !== null && instr.address !== lastAddr + instructions[i - 1].bytes) {
					console.warn('Address discontinuity detected:', lastAddr, 'to', instr.address);
				}
				lastAddr = instr.address;

				const row = document.createElement('div');
				row.className = 'instruction d-flex mb-1 py-1 px-2 rounded';
				row.dataset.address = instr.address.toString(16);

				if (instr.address === pc) {
					row.className += ' current';
				}

				if (this.breakpoints.has(instr.address)) {
					row.className += ' border-warning border-start border-3';
				}

				const addrSpan = document.createElement('span');
				addrSpan.className = 'me-3 text-primary';
				addrSpan.style.width = '60px';
				addrSpan.textContent = `$${instr.address.toString(16).toUpperCase().padStart(4, '0')}`;

				const opcodeSpan = document.createElement('span');
				opcodeSpan.className = 'me-3 text-secondary';
				opcodeSpan.style.width = '30px';
				opcodeSpan.textContent = `${instr.opcode.toString(16).toUpperCase().padStart(2, '0')}`;

				const mnemonicSpan = document.createElement('span');
				mnemonicSpan.className = 'me-3 fw-bold';
				mnemonicSpan.style.width = '50px';
				mnemonicSpan.textContent = instr.mnemonic;

				const operandsSpan = document.createElement('span');
				operandsSpan.className = '';

				// Extract operands part from formatted string
				if (instr.operand) {
					// Fallback if formatting didn't work
					operandsSpan.textContent = String(instr.operand);
				}
				else if (instr.formatted && instr.mnemonic) {
					// If formatted string includes the mnemonic, extract the rest
					if (instr.formatted.indexOf(instr.mnemonic) === 0) {
						const operandPart = instr.formatted.substring(instr.mnemonic.length).trim();
						operandsSpan.textContent = operandPart;
					} else {
						operandsSpan.textContent = instr.formatted;
					}
				}
				else {
					operandsSpan.textContent = "";
				}

				const bytesSpan = document.createElement('span');
				bytesSpan.className = 'ms-auto text-muted small';
				bytesSpan.textContent = `${instr.bytes} bytes, ${instr.cycles} cycles`;

				row.appendChild(addrSpan);
				row.appendChild(opcodeSpan);
				row.appendChild(mnemonicSpan);
				row.appendChild(operandsSpan);
				row.appendChild(bytesSpan);

				view.appendChild(row);
			}
		} catch (e) {
			console.error('Error in disassembly:', e);
			view.innerHTML = '<div class="alert alert-danger">Error disassembling code: ' + e.message + '</div>';
		}
	}

	updateMemoryView() {
		const view = document.getElementById('memoryView');
		view.innerHTML = '';

		// Add headers
		const headerRow = document.createElement('div');
		headerRow.className = 'memory-row mb-2 px-2 py-1 bg-primary bg-opacity-10 rounded';

		const addrHeader = document.createElement('div');
		addrHeader.className = 'memory-header fw-bold text-primary';
		addrHeader.style.width = '80px';
		addrHeader.textContent = 'Address';

		headerRow.appendChild(addrHeader);

		for (let i = 0; i < 16; i++) {
			const header = document.createElement('div');
			header.className = 'memory-header fw-bold text-primary';
			header.style.width = '30px';
			header.textContent = i.toString(16).toUpperCase();
			headerRow.appendChild(header);
		}

		const asciiHeader = document.createElement('div');
		asciiHeader.className = 'memory-header fw-bold text-primary ms-2';
		asciiHeader.style.width = '180px';
		asciiHeader.textContent = 'ASCII';

		headerRow.appendChild(asciiHeader);
		view.appendChild(headerRow);

		// Calculate how many rows to display
		const rowCount = Math.min(16, Math.ceil(this.memoryPageSize / 16));

		// Add memory rows
		for (let row = 0; row < rowCount; row++) {
			const rowStartAddr = this.currentMemoryPage + (row * 16);

			const memoryRow = document.createElement('div');
			memoryRow.className = 'memory-row mb-1 px-2 py-1 rounded';

			const addrCell = document.createElement('div');
			addrCell.className = 'memory-address text-primary';
			addrCell.style.width = '80px';
			addrCell.textContent = `$${rowStartAddr.toString(16).toUpperCase().padStart(4, '0')}`;

			memoryRow.appendChild(addrCell);

			let asciiText = '';

			for (let col = 0; col < 16; col++) {
				const addr = rowStartAddr + col;

				if (addr <= 0xFFFF) {
					const value = this.debugger.readMemory(addr);

					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center';
					cell.style.width = '30px';
					cell.dataset.address = addr;
					cell.textContent = value.toString(16).toUpperCase().padStart(2, '0');
					cell.setAttribute('data-bs-toggle', 'tooltip');
					cell.setAttribute('data-bs-placement', 'top');
					cell.setAttribute('title', `Click to edit memory at $${addr.toString(16).toUpperCase().padStart(4, '0')}`);

					// Highlight current PC
					if (addr === this.debugger.getRegisterPC()) {
						cell.className += ' bg-danger bg-opacity-25 fw-bold';
					}

					// Highlight SP for stack memory page
					if (this.currentMemoryPage === 0x0100 && addr === 0x0100 + this.debugger.getRegisterSP()) {
						cell.className += ' bg-success bg-opacity-25 fw-bold';
					}

					memoryRow.appendChild(cell);

					// ASCII representation
					asciiText += (value >= 32 && value <= 126) ? String.fromCharCode(value) : '.';
				} else {
					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center text-muted';
					cell.style.width = '30px';
					cell.textContent = '--';
					memoryRow.appendChild(cell);

					asciiText += ' ';
				}
			}

			const asciiCell = document.createElement('div');
			asciiCell.className = 'memory-ascii ms-2 font-monospace';
			asciiCell.style.width = '180px';
			asciiCell.textContent = asciiText;

			memoryRow.appendChild(asciiCell);
			view.appendChild(memoryRow);
		}

		// Re-initialize tooltips
		this.initTooltips();
	}

	updateStats() {
		const state = this.debugger.getState();
		if (!state) return;

		document.getElementById('instructionCount').textContent = state.stats.instructionCount.toString();
		document.getElementById('cycleCount').textContent = state.stats.cycleCount.toString();

		const isRunning = state.running;
		document.getElementById('runButton').disabled = isRunning;
		document.getElementById('stopButton').disabled = !isRunning;

		// Update button styles based on running state
		if (isRunning) {
			document.getElementById('runButton').classList.add('disabled');
			document.getElementById('stepButton').classList.add('disabled');
			document.getElementById('stopButton').classList.remove('disabled');
		} else {
			document.getElementById('runButton').classList.remove('disabled');
			document.getElementById('stepButton').classList.remove('disabled');
			document.getElementById('stopButton').classList.add('disabled');
		}
	}
}

window.debuggerUI = new DebuggerUI();
