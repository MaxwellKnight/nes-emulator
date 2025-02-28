document.addEventListener('DOMContentLoaded', function() {
	console.log('DOM content loaded, setting up initialization...');
	const initializationTimeout = setTimeout(() => {
		console.error('Debugger initialization timed out');
	}, 30000);

	window.initializeNESDebugger = function() {
		try {
			if (!window.nesDebugger) {
				window.nesDebugger = new NESDebugger();
			}

			window.nesDebugger.module = Module;
			window.nesDebugger.setupFunctions();
			window.nesDebugger.isLoaded = true;

			if (!window.debuggerUI) {
				window.debuggerUI = new DebuggerUI();
			}

			if (typeof bootstrap !== 'undefined') {
				const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
				tooltipTriggerList.forEach(tooltipTriggerEl => {
					new bootstrap.Tooltip(tooltipTriggerEl);
				});
			}

			Module.__initialized = true;
			clearTimeout(initializationTimeout);
			if (window.debuggerUI && typeof window.debuggerUI.updateUI === 'function') {
				window.debuggerUI.updateUI();
			}

			const event = new CustomEvent('debugger-ready');
			window.dispatchEvent(event);
		} catch (error) {
			if (document.getElementById('status-message')) {
				document.getElementById('status-message').textContent = 'Initialization error: ' + error.message;
			}
		}
	};

	function loadWasmModule() {
		const script = document.createElement('script');
		script.src = 'js/cpu_wasm.js';

		script.onload = function() {
			checkModuleReady();
		};

		script.onerror = function(e) {
			console.error('Failed to load WASM script', e);
		};

		document.body.appendChild(script);
	}

	function checkModuleReady() {
		if (window.CPUEmulator) {
			try {
				window.CPUEmulator().then(function(module) {
					window.Module = module;

					setTimeout(function() {
						window.initializeNESDebugger();
					}, 100);
				}).catch(function(err) {
					console.error('Error creating CPUEmulator instance:', err);
				});
			} catch (error) {
				console.error('Error initializing CPUEmulator:', error);
				setTimeout(checkModuleReady, 500);
			}
		}
		else if (window.Module && typeof window.Module.ccall === 'function') {
			setTimeout(function() {
				window.initializeNESDebugger();
			}, 100);
		}
		else {
			setTimeout(checkModuleReady, 500);
		}
	}

	loadWasmModule();
});
