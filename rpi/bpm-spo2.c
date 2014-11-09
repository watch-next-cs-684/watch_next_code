#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART




FILE *fbpm, *fspo2;


void intHandler() {
	printf("Signal rec\n");
	fclose(fbpm);
	fclose(fspo2);
	exit(0);
}

/*
 * The main program
 */
int main(int argc, char *argv[]) {
	/*************************************************
	 *
	 *	VARIABLE DECLARATION
	 *
	 **************************************************/
	/************************
	 * Used to read the buffer  from serial port
	 ************************/
	unsigned char rx_buffer[1];
	int rx_length;
	int uart0_filestream = -1;
	char zero = 0;

	/************************
	 * Used to read the buffer  from serial port
	 ************************/
	time_t rawtime;
	struct tm * timeinfo;
	char hms_buff[80];
	char dmy_buff_pulse[80];
	char dmy_buff_spo2[80];
	clock_t start_t, curr_t;

	/************************
	 * Used for different computations
	 ************************/
	long int m = 0, ctr, min, max, high, low, bpm, oxy;
	int flagIr, flagMother = 0;
	FILE *fp;
	char buff[10];
	int data[100];
	int count = 0, size = 100, prev = 0, pulse_rate = 0, pickval = 1;
	long int bit_count = 0, min1, max1, high1 = 0, low1 = 0, prev_bit_count = 0;
	struct termios options;
	double time_diff;
	int isSample = 1;
	long oxy_count = 0;
	double red_ac = 0, red_dc = 0, ir_ac = 0, ir_dc = 0;

	/************************
	 * Get the current date to be appended in the file name
	 ************************/
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(dmy_buff_pulse, 80, "%Y-%m-%d-pulse.txt", timeinfo);
	fbpm = fopen(dmy_buff_pulse, "a");
	strftime(dmy_buff_spo2, 80, "%Y-%m-%d-spo2.txt", timeinfo);
	fspo2 = fopen(dmy_buff_spo2, "a");

	/************************
	 * Check for IO issues
	 ************************/
	if (fbpm == NULL) {
		printf("Error opening bpm.txt");
		exit(1);
	}

	/************************
	 * Initializing variables
	 ************************/
	m = ctr = max = bpm = 0;
	min = 1024;
	min1 = 1024;
	max1 = 0;

	/************************
	 * Initialize the serial port reader
	 ************************/

	uart0_filestream = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY); //Open in non blocking read/write mode
	if (uart0_filestream == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		printf(
				"Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	/************************
	* End of Initialization of the serial port reader
	************************/

	start_t = clock();


	/******************************************
	 *
	 * Start calculating the Pulse Rate and SPO2
	 *
	 ******************************************/
	while (1) {
		//Check if the file stream is available from the serial port
		if (uart0_filestream != -1) {

			/*******************************************************************************
			 * 254 - signifies we are getting IR LED data, it will continue for next 30 secs
			 * 255 - signifies we are getting Red LED data, it will continue for next 30 secs
			 * Here we are waiting for the first Red/IR data to start coming
			 *******************************************************************************/
			while ((m / 4) != 254 && (m / 4) != 255) {
				memset(rx_buffer, zero, sizeof(rx_buffer));
				rx_length = read(uart0_filestream, (void*) rx_buffer, 1);
				if (rx_length <= 0) {
					continue;
				}
				m = (*rx_buffer) * 4;
			}

			/******************
			 * IR Data started
			 ******************/
			if ((m / 4) == 254) {
				flagIr = 1;
				//flagmother set means we need to start calculating the SPO2 for the next minute
				flagMother = 1;
			}
			/******************
			 * Red Data started
			 ******************/
			if ((m / 4) == 255) {
				flagIr = 0;
			}
			//Reinitialize the computation variables
			m = 0;
			bit_count = 0;
			isSample = 1;
			min1 = 1024;
			max1 = 0;

			//Data is coming through serial port start calcualtion of Pulse Rate and SPO2
			while (1) {
				//Read the buffer
				memset(rx_buffer, zero, sizeof(rx_buffer));
				rx_length = read(uart0_filestream, (void*) rx_buffer, 1);
				if (rx_length <= 0) {
					while (rx_length <= 0) {
						memset(rx_buffer, zero, sizeof(rx_buffer));
						rx_length = read(uart0_filestream, (void*) rx_buffer, 1);
					}
				}

				/********************************************************************************
				 *	We are dividing the serial data in RPi by 4 to use less bit while transfering
				 *	Here the data is again multiplied by 4 to get a 0-1024 data span
				 ********************************************************************************/
				m = (*rx_buffer) * 4;

				/********************************************************************************
				 * 1020|1016 means the next red/ir cycle is going to be started
				 * hence we need to break and reinitialize the variables
				 ********************************************************************************/
				if (m == 1020 || m == 1016)
					break;

				//Reseting the buffer
				memset(rx_buffer, zero, sizeof(rx_buffer));
				rx_length = read(uart0_filestream, (void*) rx_buffer, 1);
				if (rx_length <= 0) {
					while (rx_length <= 0) {
						memset(rx_buffer, zero, sizeof(rx_buffer));
						rx_length = read(uart0_filestream, (void*) rx_buffer,
								1);
					}
				}

				//Read data from buffer
				oxy = (*rx_buffer) * 4;

				/********************************************************************************
				 * 1020|1016 means the next red/ir cycle is going to be started
				 * hence we need to break and reinitialize the variables
				 ********************************************************************************/
				if (oxy == 1020 || oxy == 1016)
					break;

				/******************************************************
				 * The first 3000 data are used as sample to calculate the
				 * min and max value of the data. It will vary from person
				 * to person and we use these samples to find the pick
				 * between two consecutive cycles and get the Pulse Rate
				 *****************************************************/
				if (bit_count <= 3500 && isSample == 1) {
					if (bit_count > 500) {
						if (m < min1)
							min1 = m;
						else if (m > max1)
							max1 = m;
					}

					/******************************************************
					 * Sampling done calculate the high and low margin to decide the pick values
					 *****************************************************/
					if (bit_count == 3500) {
						high1 = max1 - ((max1 - min1) >> 2);
						low1 = min1 + ((max1 - min1) >> 2);
						isSample = 0;
					}
				}

				/******************************************************
				 * Now start calculating and displaying the Pulse Rate
				 *****************************************************/
				else {
					/******************************************************
					 * Check for the next pick value
					 *****************************************************/
					if (pickval == 0 && m > (int) (high1)) {

						/*** Get the time elapsed between two pick values ***/
						time_diff = (double) bit_count * 2.08;

						/*** Calculate the Pulse Rate ***/
						pulse_rate = 60000 / (int) time_diff;

						/*** Pulse Rate in range ***/
						if (pulse_rate > 40 && pulse_rate < 120) {

							//Print the Pulse Rate
							printf("Pulse Rate = %d", pulse_rate);

							//Get the Time Stamp and write to the file
							time(&rawtime);
							timeinfo = localtime(&rawtime);
							strftime(hms_buff, 80, "%H:%M:%S", timeinfo);
							fprintf(fbpm, "%s %d\n", hms_buff, pulse_rate);
						}
						/*** Pulse Rate is abnormal ***/
						else {
							//send message
						}
						pickval = 1;
						bit_count = 0;
					}
					/***
					 * Now we have entered in a downward phase.
					 * Reset the variable to start looking for the next pick
					***/
					else if (pickval == 1 && m < low1) {
						pickval = 0;
					}
				}
				bit_count++;

				/******************************************************
				 * To calculate the SPO2 we need to calculate the (RED_ac/RED_dc)/(IR_ac/IR_dc)
				 * The AC part is calculated as the RMS value of the samples received
				 * The DC part is the mean of the samples received
				 *****************************************************/
				if (flagIr == 1) {
					ir_ac += oxy * oxy;
					ir_dc += oxy;
				} else {
					red_ac += oxy * oxy;
					red_dc += oxy;
				}
			}

			//Calculating the SPO2
			if (flagIr == 0) {
				ir_ac = pow(ir_ac, 0.5);
				red_ac = pow(red_ac, 0.5);

				if (flagMother == 1) {

					//Print the SPO2
					printf("spo2 = %f\n", 98.56 * (red_ac / red_dc) / (ir_ac / ir_dc));

					//Get the Timestamp and write to the SPO2 file
					time(&rawtime);
					timeinfo = localtime(&rawtime);
					strftime(hms_buff, 80, "%H:%M:%S", timeinfo);
					fprintf(fspo2, "%s %f\n", hms_buff, 98.56 * (red_ac / red_dc) / (ir_ac / ir_dc));
				}
				//Reset the SPO2 variables
				red_ac = red_dc = ir_ac = ir_dc = 0;
			}

		}
	}
	fclose(fbpm);
	fclose(fspo2);
	return 0;
}
