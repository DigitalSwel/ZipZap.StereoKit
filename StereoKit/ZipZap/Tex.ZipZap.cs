using System;

namespace StereoKit
{
	public partial class Tex
	{
		/// <summary>Checks concrete support for an uploaded, readable 2D sampled
		/// texture. Call on the SK thread after SK.Initialize. Returns false
		/// before initialization or when the backend cannot establish support.
		/// Available in ZipZap extension builds.</summary>
		/// <param name="format">Exact native format, including color space.</param>
		/// <param name="linearFilter">Require linear sampling as well.</param>
		public static bool SupportsSampling(TexFormat format, bool linearFilter = true)
			=> NativeAPI.tex_format_supports_sampling(format, linearFilter ? 1 : 0) != 0;

		// Keep the released fork's flags-first signatures at this boundary.
		// A distinct format parameter name avoids ambiguity with upstream's
		// format-first overloads when both arguments are passed by name.
		/// <summary>Compatibility overload for the flags-first texture API.</summary>
		[Obsolete("Pass dataFormat before flags, as in SetColors(width, height, data, TexFormat.Rgba32, flags).")]
		public void SetColors<T>(int width, int height, T[] data, TexData flags, TexFormat legacyDataFormat = TexFormat.None, int priority = 10) where T : unmanaged
			=> SetColors(width, height, data, legacyDataFormat, flags, priority);

		/// <summary>Compatibility overload for the flags-first pointer API.</summary>
		[Obsolete("Pass dataFormat before flags.")]
		public void SetColors(int width, int height, IntPtr data, TexData flags, TexFormat legacyDataFormat = TexFormat.None, int priority = 10)
			=> SetColors(width, height, data, legacyDataFormat, flags, priority);

		/// <summary>Compatibility overload for flags-first layered texture data.</summary>
		[Obsolete("Pass dataFormat before flags.")]
		public void SetColors<T>(int width, int height, T[][] arrayData, int mipCount, TexData flags, TexFormat legacyDataFormat = TexFormat.None, int priority = 10) where T : unmanaged
			=> SetColors(width, height, arrayData, mipCount, legacyDataFormat, flags, priority);

		/// <summary>Compatibility overload for flags-first volume texture data.</summary>
		[Obsolete("Pass dataFormat before flags.")]
		public void SetColors<T>(int width, int height, int depth, T[] data, TexData flags, TexFormat legacyDataFormat = TexFormat.None, int priority = 10) where T : unmanaged
			=> SetColors(width, height, depth, data, legacyDataFormat, flags, priority);

		/// <summary>Compatibility overload for an explicit blocking argument.</summary>
		[Obsolete("Include TexData.Blocking in flags instead of passing blocking separately.")]
		public void SetMemory(in byte[] imageFileData, TexData flags, bool blocking, int priority = 10)
			=> SetMemory(imageFileData, blocking ? flags | TexData.Blocking : flags, priority);

		/// <summary>Compatibility name for raw texture creation.</summary>
		[Obsolete("Use Tex.FromColors(data, width, height, format, flags, type, priority).")]
		public static Tex FromData<T>(T[] data, TexFormat format, int width, int height, TexData flags = TexData.Srgb, TexType type = TexType.Image, int priority = 10) where T : unmanaged
			=> FromColors(data, width, height, format, flags, type, priority);

		/// <summary>Retains the released three-argument binary entry point.</summary>
		[Obsolete("Use the overload accepting TexData flags.")]
		public static Tex GenCubemapReflection(Tex sourceCubemap, Tex into, int maxResolution)
			=> GenCubemapReflection(sourceCubemap, into, maxResolution, TexData.None);
	}
}
