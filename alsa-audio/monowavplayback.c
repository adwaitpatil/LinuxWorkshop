#include <stdio.h>
#include <alsa/asoundlib.h>

static unsigned int rate = 44100;

int main(int argc, char *argv[])
{
	FILE  *fp;
	int dir;
	int err;
	int size;
	long loops;
	unsigned int rrate, val;
	char *buffer;
	char devname[32] = {0};
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;

	if (argc < 3) {
		printf("Usage: %s <wavefile_name> <module_name> <rate>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (argc >= 2) {
		fp = fopen(argv[1], "r");
		if (!fp) {
			printf("Could not open file %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}

	if (argc == 4) {
		rate = atoi(argv[3]);
		printf("Requested playback rate: %d\n", rate);
	}

	printf("Module Name: %s\n", argv[2]);

	if (!strcmp("colibri-vf", argv[2]))
		strcpy(devname, "default:CARD=Audio");
	else if (!strcmp("apalis-imx6", argv[2]))
		strcpy(devname, "default:CARD=imx6qapalissgtl");
	else if (!strcmp("colibri-imx6", argv[2]))
		strcpy(devname, "default:CARD=imx6qcolibrisgtl");
	else if (!strcmp("colibri-t20", argv[2]))
		strcpy(devname, "default:CARD=colibrit20wm971");
	else if (!strcmp("colibri-t30", argv[2]))
		strcpy(devname, "default:CARD=colibrit30sgtl5");

	err = snd_pcm_open(&handle, devname, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	snd_pcm_hw_params_alloca(&params);

	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
	}

	err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
                return err;
	}

	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (err < 0) {
		printf("Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}

	err = snd_pcm_hw_params_set_channels(handle, params, 1);
	if (err < 0) {
		printf("Channels count (%i) not available for playbacks: %s\n", 1, snd_strerror(err));
                return err;
	}

	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		printf("Rate %iHz not available for playback: %s\n", rrate, snd_strerror(err));
		return err;
	}

	if (rrate != rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
                return -EINVAL;
        }

	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n",
		snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	//size = frames * 4;		/* 2 bytes/sample, 2 channels */
	size = frames * 2;			/* 2 bytes/sample, 1 channel */
	buffer = (char *) malloc(size);
	if (!buffer) {
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
		fclose(fp);
		return -ENOMEM;
	}

	/* We want to loop for 5 seconds */
	err = snd_pcm_hw_params_get_period_time(params, &val, &dir);
	if (err < 0) {
		printf("Unable to get period time for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* 5 seconds in microseconds divided by period time */
	loops = 5000000 / val;

	while (1) {
		loops--;
		err = fread(buffer, 1, size, fp);
		if (err == 0) {
			fprintf(stderr, "end of file on input\n");
			break;
		} else if (err != size)
			fprintf(stderr, "short read: read %d bytes\n", err);

		err = snd_pcm_writei(handle, buffer, frames);
		if (err == -EPIPE) {
			/* EPIPE means underrun */
			fprintf(stderr, "underrun occurred\n");
			snd_pcm_prepare(handle);
		}
		else if (err < 0)
			fprintf(stderr, "error from writei: %s\n", snd_strerror(err));
		else if (err != (int)frames)
			fprintf(stderr, "short write, write %d frames\n", err);
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	fclose(fp);

	return 0;
}
