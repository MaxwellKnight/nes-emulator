function copyToClipboard(button, textToCopy) {
	if (!textToCopy) {
		console.error("No content provided to copy");
		return;
	}

	navigator.clipboard.writeText(textToCopy).then(() => {
		// Show success by changing the icon
		button.innerHTML = `
            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16"
                fill="#28a745" viewBox="0 0 16 16" class="position-absolute top-0 end-0 m-1">
                <path d="M13.854 3.646a.5.5 0 0 1 0 .708l-7 7a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L6.5 10.293l6.646-6.647a.5.5 0 0 1 .708 0z"/>
            </svg>
        `;

		// Reset after delay
		setTimeout(() => {
			button.innerHTML = `
                <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16"
                    fill="currentColor" class="bi bi-copy position-absolute top-0 end-0 m-1"
                    viewBox="0 0 16 16">
                    <path fill-rule="evenodd"
                        d="M4 2a2 2 0 0 1 2-2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2zm2-1a1 1 0 0 0-1 1v8a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1zM2 5a1 1 0 0 0-1 1v8a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1v-1h1v1a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2h1v1z" />
                </svg>
            `;
		}, 2000);
	}).catch(err => {
		console.error("Error copying text:", err);
	});
}
