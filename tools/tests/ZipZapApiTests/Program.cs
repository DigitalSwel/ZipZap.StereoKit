using System;
using System.Linq;
using global::StereoKit;

namespace ZipZap.StereoKit.Tests
{
	internal static class Program
	{
		private static int _checks;

		private static int Main()
		{
#if ZIPZAP_EXTENSIONS_TEST
			const bool extensionsExpected = true;
#else
			const bool extensionsExpected = false;
#endif
			var methods = typeof(Tex).GetMethods();
			Check(methods.Any(method => method.Name == "SupportsSampling") == extensionsExpected, "SupportsSampling build selection");
			Check(methods.Any(method => method.Name == "FromData") == extensionsExpected, "FromData compatibility selection");
			Check(methods.Any(method => method.Name == "GenCubemapReflection" && method.GetParameters().Length == 3) == extensionsExpected, "cubemap binary compatibility entry point");
			Check(methods.Count(method => method.Name == "SetColors" && method.GetParameters().Any(parameter => parameter.Name == "legacyDataFormat")) == (extensionsExpected ? 4 : 0), "flags-first compatibility entry points");
			Check(methods.Any(method => method.Name == "SetMemory" && method.GetParameters().Any(parameter => parameter.Name == "flags") && method.GetParameters().Any(parameter => parameter.ParameterType == typeof(bool))) == extensionsExpected, "blocking memory compatibility entry point");
			Check((int)TexData.CompressNone == 1 << 8, "CompressNone value");
			Check((int)TexData.CompressQuality == 1 << 9, "CompressQuality value");
			Check((int)TexData.CompressSmall == 1 << 10, "CompressSmall value");
#pragma warning disable CS0618 // These checks deliberately cover released fork names.
			Check(TexData.Uncompressed == TexData.CompressNone, "Uncompressed alias");
			Check(TexData.Quality == TexData.CompressQuality, "Quality alias");
			Check(TexData.Small == TexData.CompressSmall, "Small alias");
#pragma warning restore CS0618
			Console.WriteLine($"ZipZap API: {_checks} checks passed; extensions={extensionsExpected}");
			return 0;
		}

		private static void Check(bool value, string name)
		{
			if (!value) throw new InvalidOperationException(name);
			_checks++;
		}

#if ZIPZAP_EXTENSIONS_TEST
		// Compile-only coverage: these calls must bind without invoking native SK.
#pragma warning disable CS0618
		private static void CompileLegacyCalls(Tex texture, byte[] pixels, byte[][] layers, IntPtr pointer)
		{
			texture.SetColors(8, 8, pixels, TexData.Uncompressed);
			texture.SetColors(8, 8, pixels, TexData.Uncompressed, TexFormat.Rgba32, 0);
			texture.SetColors(8, 8, pointer, TexData.Uncompressed, TexFormat.Rgba32, 0);
			texture.SetColors(8, 8, layers, 1, TexData.Uncompressed, TexFormat.Rgba32, 0);
			texture.SetColors(8, 8, 1, pixels, TexData.Uncompressed, TexFormat.Rgba32, 0);
			texture.SetColors(8, 8, pixels, flags: TexData.Uncompressed);
			texture.SetColors(8, 8, pixels, flags: TexData.CompressNone, dataFormat: TexFormat.Rgba32);
			texture.SetMemory(pixels, TexData.Srgb, true, 0);
			Tex.FromData(pixels, TexFormat.Rgba32, 8, 8);
			Tex.GenCubemapReflection(texture, null, 64);
		}
#pragma warning restore CS0618
#endif
	}
}
