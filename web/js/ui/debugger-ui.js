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
		window.addEventListener('nes-breakpoint-hit', () => {
			this.stop();
			this.showToast('Breakpoint hit', 'warning');
		});

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
				const address = parseInt(cell.dataset.address, 10);
				this.promptEditMemoryValue(address);
			}
		});

		window.addEventListener('nes-brk-encountered', () => {
			this.stop();
			this.showToast('Program terminated with BRK instruction', 'info');
		});

		this.initTooltips();
	}

	initTooltips() {
		const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
		[...tooltipTriggerList].forEach(tooltipTriggerEl => {
			const tooltip = bootstrap.Tooltip.getInstance(tooltipTriggerEl);
			if (tooltip) {
				tooltip.dispose();
			}
		});

		[...tooltipTriggerList].forEach(tooltipTriggerEl => {
			new bootstrap.Tooltip(tooltipTriggerEl, {
				trigger: 'hover focus',
				html: true,
				container: 'body',     // Places tooltips in the body to avoid containment issues
				animation: false,      // Improves performance by disabling animations
				delay: { show: 200, hide: 100 } // Adds slight delay to prevent flickering
			});
		});
	}

	resetTooltips() {
		const tooltipElements = document.querySelectorAll('[data-bs-toggle="tooltip"]');
		[...tooltipElements].forEach(el => {
			const tooltip = bootstrap.Tooltip.getInstance(el);
			if (tooltip) {
				tooltip.dispose();
			}
		});

		if (!this.uiDisabled) {
			[...tooltipElements].forEach(el => {
				new bootstrap.Tooltip(el, {
					trigger: 'hover focus',
					html: true,
					container: 'body',
					animation: false,
					delay: { show: 200, hide: 100 }
				});
			});
		}
	}

	reset() {
		this.debugger.reset();
		this.updateUI();
		this.showToast('CPU reset successfully', 'success');
	}

	step() {
		this.debugger.stepInstruction();
	}

	run() {
		document.getElementById('runButton').classList.add('disabled');
		document.getElementById('stepButton').classList.add('disabled');
		document.getElementById('stopButton').classList.remove('disabled');
		this.setControlsDisabled(true);

		this.showToast('Execution started', 'info');
		this.debugger.startContinuousExecution();
	}


	stop() {
		this.setControlsDisabled(false);

		document.getElementById('runButton').classList.remove('disabled');
		document.getElementById('stepButton').classList.remove('disabled');
		document.getElementById('stopButton').classList.add('disabled');

		const disassemblyView = document.getElementById('disassemblyView');
		disassemblyView.classList.remove('interaction-disabled');

		const memoryView = document.getElementById('memoryView');
		memoryView.classList.remove('interaction-disabled');

		document.querySelectorAll('.btn').forEach(btn => {
			btn.classList.remove('disabled');
		});

		this.updateUI();
		this.initTooltips();
		this.debugger.stopContinuousExecution();
	}

	setControlsDisabled(disabled) {
		this.uiDisabled = disabled;

		const memoryView = document.getElementById('memoryView');
		const disassemblyView = document.getElementById('disassemblyView');

		if (disabled) {
			memoryView.classList.add('interaction-disabled');
			disassemblyView.classList.add('interaction-disabled');
		} else {
			memoryView.classList.remove('interaction-disabled');
			disassemblyView.classList.remove('interaction-disabled');
		}

		const inputElements = [
			'breakpointAddress',
			'memoryAddress',
			'opcodeInput',
			'memoryPage'
		];

		inputElements.forEach(id => {
			const element = document.getElementById(id);
			if (element) element.disabled = disabled;
		});

		const buttonElements = [
			'resetButton',
			'stepButton',
			'addBreakpointButton',
			'jumpToAddressButton',
			'loadRomButton',
			'loadOpcodesButton'
		];

		buttonElements.forEach(id => {
			const element = document.getElementById(id);
			if (element) {
				element.disabled = disabled;
				element.classList.toggle('disabled', disabled);
			}
		});

		if (disabled) {
			document.getElementById('runButton').classList.add('disabled');
			document.getElementById('runButton').disabled = true;
			document.getElementById('stepButton').classList.add('disabled');
			document.getElementById('stepButton').disabled = true;
			document.getElementById('stopButton').classList.remove('disabled');
			document.getElementById('stopButton').disabled = false;
		} else {
			document.getElementById('runButton').classList.remove('disabled');
			document.getElementById('runButton').disabled = false;
			document.getElementById('stepButton').classList.remove('disabled');
			document.getElementById('stepButton').disabled = false;
			document.getElementById('stopButton').classList.add('disabled');
			document.getElementById('stopButton').disabled = true;
		}

		if (!document.getElementById('interaction-disabled-style')) {
			const style = document.createElement('style');
			style.id = 'interaction-disabled-style';
			style.textContent = `
            .interaction-disabled {
                pointer-events: none !important;
                opacity: 0.7;
            }
            .btn.disabled {
                pointer-events: none !important;
                opacity: 0.65;
            }
        `;
			document.head.appendChild(style);
		}
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

		const breakpointsList = document.getElementById('breakpointsList');
		if (breakpointsList) {
			const buttons = breakpointsList.querySelectorAll(`button[data-bs-toggle="tooltip"]`);
			buttons.forEach(button => {
				const tooltip = bootstrap.Tooltip.getInstance(button);
				if (tooltip) {
					tooltip.dispose();
				}
			});
		}

		this.updateBreakpointsList();
		this.updateDisassembly();
		this.showToast(`Breakpoint removed from $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'info');
	}

	toggleBreakpoint(addr) {
		if (this.uiDisabled) return;

		if (this.breakpoints.has(addr)) {
			this.removeBreakpoint(addr);
		} else {
			this.debugger.addBreakpoint(addr);
			this.breakpoints.add(addr);
			this.updateBreakpointsList();
			this.updateDisassembly();
			this.showToast(`Breakpoint toggled at $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'success');

			this.resetTooltips();
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

	changeMemoryPage(toast = true) {
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
		if (toast)
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
			this.changeMemoryPage(false);
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
		const state = this.debugger.getState();
		const isRunning = state && state.running;

		this.updateRegisters();
		this.updateFlags();
		this.updateDisassembly();
		this.updateMemoryView();
		this.updateStats();

		if (!isRunning) {
			this.initTooltips();
		}

		this.setControlsDisabled(isRunning);
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
		let nextAddr = null;

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

			const addr = parseIntSafe(parts[0]);
			const opcode = parseIntSafe(parts[1]);
			const mnemonic = parts[2] || 'UNK';
			const operand = parseIntSafe(parts[3], 0);
			const formatted = parts[4] || mnemonic;
			const bytes = parseIntSafe(parts[5], 1);
			const cycles = parseIntSafe(parts[6], 0);

			// If we got a weird address but we know what it should be, fix it
			const finalAddr = isNaN(addr) || addr <= 0 ? nextAddr : addr;

			const instr = {
				address: finalAddr,
				opcode: opcode,
				mnemonic: mnemonic,
				operand: operand,
				formatted: formatted,
				bytes: bytes,
				cycles: cycles
			};

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

	disassembleAroundPC(before, after) {
		const rawData = Module.ccall(
			'debugger_disassemble_around_pc',
			'string',
			['number', 'number'],
			[before, after]
		);

		return parseDisassemblyInstructions(rawData);
	}

	disassembleRange(start, end) {
		const rawData = Module.ccall(
			'debugger_disassemble_range',
			'string',
			['number', 'number'],
			[start, end]
		);

		return parseDisassemblyInstructions(rawData);
	}

	updateDisassembly() {
		const view = document.getElementById('disassemblyView');
		view.innerHTML = '';

		try {
			const instructions = this.debugger.disassembleAroundPC(5, 30);
			const pc = this.debugger.getRegisterPC();

			const disassemblyContainer = document.createElement('div');
			disassemblyContainer.className = 'disassembly-container';

			let lastAddr = null;
			for (let i = 0; i < instructions.length; i++) {
				const instr = instructions[i];

				if (lastAddr !== null && instr.address !== lastAddr + instructions[i - 1].bytes) {
					const gap = instr.address - (lastAddr + instructions[i - 1].bytes);
					if (gap > 3) {
						console.warn(`Address discontinuity detected: 0x${lastAddr.toString(16).toUpperCase()} to 0x${instr.address.toString(16).toUpperCase()} (gap: ${gap} bytes)`);
					}
				}

				lastAddr = instr.address;

				const row = document.createElement('div');
				row.className = 'instruction row g-2 mb-1 py-1 px-2 rounded align-items-center';
				row.dataset.address = instr.address.toString(16);

				if (instr.address === pc) {
					row.classList.add('current');
				}

				if (this.breakpoints.has(instr.address)) {
					row.classList.add('border', 'border-warning', 'border-start', 'border-3');
				}

				// Address column
				const addrCol = document.createElement('div');
				addrCol.className = 'col-1 pe-3 text-primary instruction-address';
				addrCol.textContent = `$${instr.address.toString(16).toUpperCase().padStart(4, '0')}`;
				row.appendChild(addrCol);

				// Opcode bytes (can be 1-3 bytes)
				const opcodeCol = document.createElement('div');
				opcodeCol.className = 'col-2 px-2 text-secondary instruction-opcode d-none d-md-block';

				// Format opcode bytes: opcode + possible operand bytes
				let opcodeBytes = `0x${instr.opcode.toString(16).toUpperCase().padStart(2, '0')}`;
				if (instr.bytes > 1) {
					if (instr.bytes === 2) {
						// For 2-byte instructions, add low byte
						opcodeBytes += ' 0x' + (instr.operand & 0xFF).toString(16).toUpperCase().padStart(2, '0');
					} else if (instr.bytes === 3) {
						// For 3-byte instructions, add low and high bytes
						opcodeBytes += ' 0x' + (instr.operand & 0xFF).toString(16).toUpperCase().padStart(2, '0');
						opcodeBytes += ' 0x' + ((instr.operand >> 8) & 0xFF).toString(16).toUpperCase().padStart(2, '0');
					}
				}
				opcodeCol.textContent = opcodeBytes;
				row.appendChild(opcodeCol);

				// Mnemonic column
				const mnemonicCol = document.createElement('div');
				mnemonicCol.className = 'col-1 px-2 fw-bold instruction-mnemonic';
				mnemonicCol.textContent = instr.mnemonic;
				row.appendChild(mnemonicCol);

				// Operands column with proper formatting
				const operandsCol = document.createElement('div');
				operandsCol.className = 'col instruction-operands';

				// Format operand with correct addressing mode notation
				let operandText = this.formatOperand(instr);
				operandsCol.innerHTML = operandText; // Using innerHTML to support formatting
				row.appendChild(operandsCol);

				// Details column (bytes and cycles)
				const detailsCol = document.createElement('div');
				detailsCol.className = 'col-auto text-muted small instruction-details';
				detailsCol.textContent = `${instr.bytes} B, ${instr.cycles} cyc`;
				row.appendChild(detailsCol);

				// Mobile view
				const mobileCompactView = document.createElement('div');
				mobileCompactView.className = 'd-md-none instruction-mobile-compact small text-muted';
				mobileCompactView.innerHTML = `
                <span class="text-primary me-1">${instr.address.toString(16).toUpperCase().padStart(4, '0')}</span>
                <span class="text-secondary me-1">0x${instr.opcode.toString(16).toUpperCase().padStart(2, '0')}</span>
                <span class="fw-bold me-1">${instr.mnemonic}</span>
                <span>${operandText}</span>
                <span class="ms-auto">(${instr.bytes}B, ${instr.cycles}cyc)</span>
            `;
				row.appendChild(mobileCompactView);

				disassemblyContainer.appendChild(row);
			}

			view.appendChild(disassemblyContainer);

		} catch (e) {
			console.error('Error in disassembly:', e);
			view.innerHTML = `
            <div class="alert alert-danger">
                <strong>Disassembly Error:</strong> ${e.message}
            </div>
        `;
		}
	}

	toHex(value, digits = 2) {
		return value.toString(16).toUpperCase().padStart(digits, '0');
	}

	formatOperand(instr) {
		if (!instr.operand && !instr.formatted) {
			return ""; // No operand (implied addressing)
		}

		// If there's a formatted string from the disassembler, use that as a starting point
		let operandText = "";
		if (instr.formatted && instr.mnemonic) {
			if (instr.formatted.indexOf(instr.mnemonic) === 0) {
				operandText = instr.formatted.substring(instr.mnemonic.length).trim();
			} else {
				operandText = instr.formatted;
			}
		}

		// If we have a raw operand value but no formatted text, format it based on common 6502 addressing modes
		if (instr.operand && (!operandText || operandText === String(instr.operand))) {
			const operand = instr.operand;
			// Try to infer addressing mode from instruction bytes and opcode
			switch (instr.bytes) {
				case 1:
					// Implied or Accumulator addressing
					return ""; // No operand needed

				case 2:
					// Immediate, Zero Page, Zero Page X/Y, Relative addressing
					// Check for common immediate mode opcodes (LDA #$xx, LDX #$xx, etc.)
					if (instr.opcode === 0xA9 || instr.opcode === 0xA2 || instr.opcode === 0xA0 ||
						instr.opcode === 0xC9 || instr.opcode === 0xE0 || instr.opcode === 0xC0) {
						// Immediate addressing
						return `<span class="text-info">#0x${(operand & 0xFF).toString(16).toUpperCase().padStart(2, '0')}</span>`;
					}

					// Check for branches (BEQ, BNE, etc.) which use relative addressing
					if ((instr.opcode & 0x1F) === 0x10) {
						// Relative addressing - calculate target address
						const offset = operand & 0xFF;
						const target = instr.address + 2 + ((offset < 128) ? offset : offset - 256);
						return `<span class="text-success">$${target.toString(16).toUpperCase().padStart(4, '0')}</span>`;
					}

					// Assume Zero Page for other 2-byte instructions
					return `<span class="text-warning">$${(operand & 0xFF).toString(16).toUpperCase().padStart(2, '0')}</span>`;

				case 3:
					// Absolute, Absolute X/Y, Indirect addressing
					return `<span class="text-success">$${operand.toString(16).toUpperCase().padStart(4, '0')}</span>`;
			}
		}

		// If we have a formatted string but it doesn't have proper hex notation, add it
		if (operandText) {
			// Convert all hex notation to use 0x format except for $ addresses and # immediate values
			operandText = operandText.replace(/\$([0-9A-F]{2,4})/gi, (_, hex) => {
				// Keep $ for addresses, but ensure only one $
				return `$${hex}`;
			});

			operandText = operandText.replace(/#\$([0-9A-F]{2})/gi, (_, hex) => {
				// Change #$ to # with 0x for immediate values
				return `#0x${hex}`;
			});

			operandText = operandText.replace(/\b(\d+)\b/g, (match, number) => {
				const num = parseInt(number, 10);
				return num <= 0xFF ? `0x${this.toHex(num)}` : match; // Convert only if it's 8-bit
			});

			// Add color formatting based on addressing mode
			if (operandText.includes('#')) {
				// Immediate addressing
				return `<span class="text-info">${operandText}</span>`;
			} else if (operandText.includes(',')) {
				// Indexed addressing
				return `<span class="text-warning">${operandText}</span>`;
			} else if (operandText.includes('(') && operandText.includes(')')) {
				// Indirect addressing
				return `<span class="text-danger">${operandText}</span>`;
			} else if (operandText.length <= 5 && operandText.startsWith('$')) { // $xx or $xxxx
				// Zero page or absolute addressing
				return `<span class="text-success">${operandText}</span>`;
			}
		}

		return operandText;
	}

	updateMemoryView() {
		const view = document.getElementById('memoryView');
		view.innerHTML = '';
		view.classList.add('container-fluid', 'px-0');

		const headerRow = document.createElement('div');
		headerRow.className = 'memory-row row g-0 mb-2 px-2 py-1 bg-primary bg-opacity-10 rounded align-items-center';

		const addrHeader = document.createElement('div');
		addrHeader.className = 'memory-header col-2 col-md-1 fw-bold text-primary text-truncate';
		addrHeader.textContent = 'Addr';
		headerRow.appendChild(addrHeader);

		const hexHeaderContainer = document.createElement('div');
		hexHeaderContainer.className = 'col-auto d-none d-md-flex flex-nowrap';
		for (let i = 0; i < 16; i++) {
			const header = document.createElement('div');
			header.className = 'memory-header text-center fw-bold text-primary';
			header.style.width = '30px';
			header.style.minWidth = '30px';
			header.style.maxWidth = '30px';
			header.textContent = i.toString(16).toUpperCase();
			hexHeaderContainer.appendChild(header);
		}
		headerRow.appendChild(hexHeaderContainer);

		const asciiHeader = document.createElement('div');
		asciiHeader.className = 'memory-header col-10 col-md-4 fw-bold text-primary text-truncate';
		asciiHeader.textContent = 'ASCII';
		headerRow.appendChild(asciiHeader);

		view.appendChild(headerRow);

		const rowCount = Math.min(16, Math.ceil(this.memoryPageSize / 16));

		for (let row = 0; row < rowCount; row++) {
			const rowStartAddr = this.currentMemoryPage + (row * 16);
			const memoryRow = document.createElement('div');
			memoryRow.className = 'memory-row row g-0 mb-1 px-2 py-1 rounded align-items-center';
			// Add debug attribute for inspecting
			memoryRow.dataset.rowAddress = rowStartAddr.toString(16).toUpperCase().padStart(4, '0');

			const addrCell = document.createElement('div');
			addrCell.className = 'memory-address col-2 col-md-1 text-primary text-truncate';
			addrCell.textContent = `$${rowStartAddr.toString(16).toUpperCase().padStart(4, '0')}`;
			memoryRow.appendChild(addrCell);

			let asciiText = '';
			const hexCells = document.createElement('div');
			hexCells.className = 'col-auto d-none d-md-flex flex-nowrap';

			for (let col = 0; col < 16; col++) {
				const addr = rowStartAddr + col;
				if (addr <= 0xFFFF) {
					const value = this.debugger.readMemory(addr);

					// Create a cell with a consistent format for the data-address attribute
					// IMPORTANT: Store as a decimal string since that's how dataset attributes work
					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center';
					cell.style.width = '30px';
					cell.style.minWidth = '30px';
					cell.style.maxWidth = '30px';
					cell.classList.add('col-auto', 'd-none', 'd-md-block');

					// Store address as decimal in data attribute
					cell.dataset.address = addr.toString(10); // Explicitly store as decimal string

					// Add custom attributes for debugging
					cell.dataset.addressHex = addr.toString(16).toUpperCase().padStart(4, '0');
					cell.dataset.row = row;
					cell.dataset.col = col;

					cell.textContent = value.toString(16).toUpperCase().padStart(2, '0');
					cell.setAttribute('data-bs-toggle', 'tooltip');
					cell.setAttribute('data-bs-placement', 'top');
					cell.setAttribute('title', `Click to edit memory at $${addr.toString(16).toUpperCase().padStart(4, '0')}`);

					if (addr === this.debugger.getRegisterPC()) {
						cell.classList.add('bg-danger', 'bg-opacity-25', 'fw-bold');
					}

					if (this.currentMemoryPage === 0x0100 && addr === 0x0100 + this.debugger.getRegisterSP()) {
						cell.classList.add('bg-success', 'bg-opacity-25', 'fw-bold');
					}

					hexCells.appendChild(cell);

					asciiText += (value >= 32 && value <= 126) ? String.fromCharCode(value) : '.';
				} else {
					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center text-muted col-auto d-none d-md-block';
					cell.style.width = '30px';
					cell.style.minWidth = '30px';
					cell.style.maxWidth = '30px';
					cell.textContent = '--';
					hexCells.appendChild(cell);
					asciiText += ' ';
				}
			}

			memoryRow.appendChild(hexCells);

			const mobileHexView = document.createElement('div');
			mobileHexView.className = 'd-md-none col-auto mb-2';
			mobileHexView.innerHTML = `<small class="text-muted">${Array.from({ length: 16 }, (_, col) => {
				const addr = rowStartAddr + col;
				return addr <= 0xFFFF
					? this.debugger.readMemory(addr).toString(16).toUpperCase().padStart(2, '0')
					: '--'
			}).join(' ')}</small>`;
			memoryRow.appendChild(mobileHexView);

			const asciiCell = document.createElement('div');
			asciiCell.className = 'memory-ascii col-10 col-md-4 font-monospace text-truncate';
			asciiCell.textContent = asciiText;
			memoryRow.appendChild(asciiCell);

			view.appendChild(memoryRow);
		}

		this.initTooltips();
	}

	promptEditMemoryValue(address) {
		address = Number(address);

		const currentValue = this.debugger.readMemory(address);
		const formattedAddress = address.toString(16).toUpperCase().padStart(4, '0');
		const formattedValue = currentValue.toString(16).toUpperCase().padStart(2, '0');

		let existingModal = document.getElementById('memoryEditModal');
		if (existingModal) {
			existingModal.remove();
		}

		const modalHtml = `
        <div class="modal fade" id="memoryEditModal" tabindex="-1" aria-labelledby="memoryEditModalLabel" aria-hidden="true">
            <div class="modal-dialog modal-dialog-centered">
                <div class="modal-content">
                    <div class="modal-header">
                        <h5 class="modal-title" id="memoryEditModalLabel">Edit Memory Address: $${formattedAddress}</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-info">
                            <small>
                                Editing memory at address $${formattedAddress} (${address} decimal)<br>
                                Memory page: $${(address & 0xFF00).toString(16).toUpperCase().padStart(4, '0')}<br>
                                Offset: $${(address & 0xFF).toString(16).toUpperCase().padStart(2, '0')}
                            </small>
                        </div>
                        <form id="memoryEditForm">
                            <div class="mb-3">
                                <label for="newMemoryValue" class="form-label">Value (Hexadecimal)</label>
                                <div class="input-group">
                                    <span class="input-group-text">$</span>
                                    <input type="text" class="form-control" id="newMemoryValue" 
                                           value="${formattedValue}" maxlength="2" 
                                           pattern="[0-9A-Fa-f]{1,2}" required>
                                </div>
                                <div class="form-text">Enter a hex value between 00 and FF</div>
                            </div>
                            <div class="row mb-3">
                                <div class="col">
                                    <label class="form-label">Decimal</label>
                                    <input type="number" class="form-control" id="decimalValue" 
                                           value="${currentValue}" min="0" max="255">
                                </div>
                                <div class="col">
                                    <label class="form-label">Binary</label>
                                    <input type="text" class="form-control" id="binaryValue" 
                                           value="${currentValue.toString(2).padStart(8, '0')}" disabled>
                                </div>
                            </div>
                        </form>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="button" class="btn btn-primary" id="saveMemoryValue">Save</button>
                    </div>
                </div>
            </div>
        </div>
    `;

		const modalContainer = document.createElement('div');
		modalContainer.innerHTML = modalHtml;
		document.body.appendChild(modalContainer.firstElementChild);

		const modal = new bootstrap.Modal(document.getElementById('memoryEditModal'));

		modal.show();

		const hexInput = document.getElementById('newMemoryValue');
		const decInput = document.getElementById('decimalValue');
		const binInput = document.getElementById('binaryValue');

		hexInput.addEventListener('input', () => {
			const hexVal = hexInput.value.trim();
			if (/^[0-9A-Fa-f]{1,2}$/.test(hexVal)) {
				const decVal = parseInt(hexVal, 16);
				decInput.value = decVal;
				binInput.value = decVal.toString(2).padStart(8, '0');
			}
		});

		decInput.addEventListener('input', () => {
			const decVal = parseInt(decInput.value);
			if (!isNaN(decVal) && decVal >= 0 && decVal <= 255) {
				hexInput.value = decVal.toString(16).toUpperCase().padStart(2, '0');
				binInput.value = decVal.toString(2).padStart(8, '0');
			}
		});

		// Auto-focus the input field
		hexInput.focus();
		hexInput.select();

		const self = this;
		const targetAddress = address;

		document.getElementById('saveMemoryValue').addEventListener('click', function() {
			const hexVal = hexInput.value.trim();
			if (/^[0-9A-Fa-f]{1,2}$/.test(hexVal)) {
				const newValue = parseInt(hexVal, 16);

				try {
					self.debugger.writeMemory(targetAddress, newValue);
					const checkValue = self.debugger.readMemory(targetAddress);

					const memoryCell = document.querySelector(`.memory-cell[data-address="${targetAddress}"]`);
					if (memoryCell) {
						memoryCell.textContent = checkValue.toString(16).toUpperCase().padStart(2, '0');

						// Highlight the updated cell
						memoryCell.classList.add('bg-warning');
						setTimeout(() => {
							memoryCell.classList.remove('bg-warning');
							memoryCell.classList.add('bg-success', 'bg-opacity-25');
							setTimeout(() => {
								memoryCell.classList.remove('bg-success', 'bg-opacity-25');
							}, 500);
						}, 1000);
						self.updateDisassembly();
					} else {
						self.updateMemoryView();
					}

					// Update ASCII representation if needed
					const rowAddress = targetAddress & 0xFFF0; // Get the row start address
					const rowOffset = targetAddress & 0x000F; // Get position in row

					const asciiCell = document.querySelector(`.memory-row[data-row-address="${rowAddress.toString(16).toUpperCase().padStart(4, '0')}"] .memory-ascii`);
					if (asciiCell) {
						const asciiText = asciiCell.textContent;
						const newChar = (checkValue >= 32 && checkValue <= 126) ? String.fromCharCode(checkValue) : '.';
						if (asciiText.length === 16 && rowOffset < 16) {
							const newText = asciiText.substring(0, rowOffset) + newChar + asciiText.substring(rowOffset + 1);
							asciiCell.textContent = newText;
						}
					}

					self.showToast(`Memory at $${formattedAddress} updated to $${newValue.toString(16).toUpperCase().padStart(2, '0')}`, 'success');

					modal.hide();
				} catch (error) {
					console.error('Exception during memory write:', error);
					self.showToast(`Error updating memory: ${error.message}`, 'danger');
				}
			} else {
				hexInput.classList.add('is-invalid');
				self.showToast('Invalid byte value (must be 00-FF)', 'danger');
			}
		});

		document.getElementById('memoryEditForm').addEventListener('submit', function(e) {
			e.preventDefault();
			document.getElementById('saveMemoryValue').click();
		});

		document.getElementById('memoryEditModal').addEventListener('hidden.bs.modal', function() {
			this.remove();
		});
	}

	reloadMemoryView() {
		const currentPage = this.currentMemoryPage;

		this.currentMemoryPage = (currentPage === 0) ? 0x100 : 0;
		this.updateMemoryView();

		this.currentMemoryPage = currentPage;
		this.updateMemoryView();
		this.showToast("Memory view refreshed", "info");
	}

	updateStats() {
		const state = this.debugger.getState();
		if (!state) return;

		document.getElementById('instructionCount').textContent = state.stats.instructionCount.toString();
		document.getElementById('cycleCount').textContent = state.stats.cycleCount.toString();

		const isRunning = state.running;
		document.getElementById('runButton').disabled = isRunning;
		document.getElementById('stopButton').disabled = !isRunning;

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
