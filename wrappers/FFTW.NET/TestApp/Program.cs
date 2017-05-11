using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Numerics;
using FFTW.NET;

namespace TestApp
{
	class Program
	{
		static void Main(string[] args)
		{
			/// Make sure either libfftw3-3-x86.dll or libfftw3-3-x64.dll
			/// (according to <see cref="Environment.Is64BitProcess"/>)
			/// is in the output directory

			DFT.Wisdom.Import("wisdom.txt");

			Example1D();

			Console.ReadKey();
			Console.Clear();

			Example2D();

			Console.ReadKey();
			Console.Clear();

			ExampleR2C();

			Console.ReadKey();
			Console.Clear();

			DFT.Wisdom.Export("wisdom.txt");
			Console.WriteLine(DFT.Wisdom.Current);
			Console.ReadKey();
		}

		static void Example1D()
		{
			Complex[] input = new Complex[2048];
			Complex[] output = new Complex[input.Length];

			for (int i = 0; i < input.Length; i++)
				input[i] = Math.Sin(i * 2 * Math.PI * 128 / input.Length);

			DFT.FFT(new Array<Complex>(input), new Array<Complex>(output));
			DFT.IFFT(new Array<Complex>(output), new Array<Complex>(output));

			for (int i = 0; i < input.Length; i++)
				Console.WriteLine(output[i] / input[i]);
		}

		static void Example2D()
		{
			Complex[,] input = new Complex[64,16];
			Complex[,] output = new Complex[input.GetLength(0), input.GetLength(1)];

			for (int row = 0; row < input.GetLength(0); row++)
			{
				for (int col = 0; col < input.GetLength(1); col++)
					input[row, col] = (double)row * col/input.Length;
			}

			DFT.FFT(new Array<Complex>(input), new Array<Complex>(output));

			for (int row = 0; row < input.GetLength(0); row++)
			{
				for (int col = 0; col < input.GetLength(1); col++)
					Console.Write((output[row, col].Real/Math.Sqrt(input.Length)).ToString("F2").PadLeft(6));
				Console.WriteLine();
			}
		}

		static void ExampleR2C()
		{
			double[] input = new double[97];
			Array<double> wInput = new Array<double>(input);
			Array<Complex> wOutput = new Array<Complex>(DFT.GetComplexBufferSize(wInput.GetSize()));
			Complex[] output = wOutput.Buffer as Complex[];

			var rand = new Random();

			for (int i = 0; i < input.Length; i++)
				input[i] = rand.NextDouble();

			DFT.FFT(wInput, wOutput);

			for (int i = 0; i < output.Length; i++)
				Console.WriteLine(output[i]);
		}
	}
}
