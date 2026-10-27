using System.Runtime.InteropServices;

namespace StereoKit
{
	internal static partial class NativeAPI
	{
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern int tex_format_supports_sampling(TexFormat format, int linear_filter);
	}
}
