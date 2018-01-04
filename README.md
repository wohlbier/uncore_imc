This is some code I got second hand for reading uncore imc counters. I have
modified it to read broadwell uncore imc for testing and verification purposes.

Note one needs a PAPI that supports uncore on Broadwell. As of now this can
be obtained by cloning the papi git repository. See tau.sh for how to point
TAU Commander to your special PAPI.

Using TAU derived metric in paraprof got 11991 MiB/s, as compared to the
code reading counters got 11461 MiB/s. <~ 5%.
