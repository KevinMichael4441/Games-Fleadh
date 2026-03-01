[Back to main README](../README.md)


### Deploy on R36S <a name="deploying-to-r36s"></a>

- After building, copy the compiled binary gpp_r36s to your R36S device.
- Store the binary in `ROOT/home/ark/autostart/` folder on R36S microSD card.
- To run from the terminal:

	```bash
	./gpp_r36s
	```

- To exit the Docker container:

	```bash
	exit
	```

- If you accidentally delete the build directory, rebuild the container with:

	```bash
	sudo docker build --no-cache -t r36s-raylib .
	```

[Back to main README](../README.md)