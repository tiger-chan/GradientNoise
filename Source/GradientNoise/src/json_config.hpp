#ifndef GRADIENT_NOISE_SRC_JSON_CONFIG
#define GRADIENT_NOISE_SRC_JSON_CONFIG

#include "CoreMinimal.h"
#include "Json.h"
#include "uproar/tasks.hpp"
#include "uproar/tasks/config.hpp"
#include <string>

namespace tc
{
namespace task
{
template <> struct config_callback<FJsonValue> {
	task_source operator()(const FJsonValue &val) const
	{
		if (val.Type == EJson::Number) {
			return val.AsNumber();
		}

		auto n = std::string{ TCHAR_TO_UTF8( *val.AsString() ) };
		auto task = tasks->find(n);

		if (task != std::end(*tasks)) {
			return task->second.get();
		}

		return 0.0;
	}

	std::unordered_map<std::string, scope_ptr<base_task>> *tasks{ nullptr };
};

template <> struct config<FJsonObject, noise_config> {
	template <typename Callback>
	void operator()(noise_config &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString octaves_key{ TEXT("octaves") };
		static const FString lacunarity_key{ TEXT("lacunarity") };
		static const FString persistance_key{ TEXT("persistance") };
		static const FString frequency_key{ TEXT("frequency") };
		static const FString amplitude_key{ TEXT("amplitude") };

		if (obj.HasField(octaves_key)) {
			task.octaves = static_cast<octave_t>(obj.GetIntegerField(octaves_key));
		}

		if (obj.HasField(lacunarity_key)) {
			task.lacunarity = obj.GetNumberField(lacunarity_key);
		}

		if (obj.HasField(persistance_key)) {
			task.persistance = obj.GetNumberField(persistance_key);
		}

		if (obj.HasField(frequency_key)) {
			task.frequency = obj.GetNumberField(frequency_key);
		}

		if (obj.HasField(amplitude_key)) {
			task.amplitude = obj.GetNumberField(amplitude_key);
		}
	}
};

template <> struct config<FJsonObject, ridged_multi_config> {
	template <typename Callback>
	void operator()(ridged_multi_config &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString exponent_key{ TEXT("exponent") };
		static const FString offset_key{ TEXT("exponent") };

		if (obj.HasField(exponent_key)) {
			task.exponent = obj.GetNumberField(exponent_key);
		}

		if (obj.HasField(offset_key)) {
			task.offset = obj.GetNumberField(offset_key);
		}

		config<FJsonObject, noise_config>{}(task, obj, callback);
	}
};

template <typename Type> struct config<FJsonObject, accumulator<Type>> {
	template <typename Callback>
	void operator()(accumulator<Type> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source_") };

		auto end = std::end(obj);
		for (size_t i = 0; i < task.size(); ++i) {
			FString key = source_key + FString::FromInt(i);

			if (obj.HasField(key)) {
				auto src = callback(*obj.GetField(key));
				task.set_source(i, src);
			}
		}
	}
};

template <> struct config<FJsonObject, bias_task> {
	template <typename Callback>
	void operator()(bias_task &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.source(src);
		}

		if (obj.HasField(bias_key)) {
			auto src = callback(*obj.GetField(bias_key));
			task.bias(src);
		}
	}
};

template <typename Noise> struct config<FJsonObject, billowing<Noise>> {
	template <typename Callback>
	void operator()(billowing<Noise> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto config = task.config();
		config<FJsonObject, noise_config>{}(config, obj, callback);
		task.set_config(config);
	}
};

template <> struct config<FJsonObject, cache> {
	template <typename Callback>
	void operator()(cache &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}
	}
};

template <> struct config<FJsonObject, gradient> {
	template <typename Callback>
	void operator()(gradient &task, const FJsonObject &obj, Callback &callback) const
	{
		std::array<decimal_t, defaults::gradient_max_dimensions> l{};
		std::array<decimal_t, defaults::gradient_max_dimensions> r{};
		l.fill(0.0);
		r.fill(0.0);

		auto end = std::end(obj);
		for (auto i = 0; i < defaults::gradient_max_dimensions; ++i) {
			FString var{ UTF8_TO_TCHAR(math::to_c_str(static_cast<math::variable>(i))) };
			auto var1 = var + "1";
			auto var2 = var + "2";

			if (obj.HasField(var1)) {
				auto src = obj.GetNumberField(var1);
				l[i] = src;
			}

			if (obj.HasField(var2)) {
				auto src = obj.GetNumberField(var2);
				r[i] = src;
			}
		}

		task.set(l, r);
	}
};

template <> struct config<FJsonObject, map_range> {
	template <typename Callback>
	void operator()(map_range &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString min_key{ TEXT("min") };
		static const FString max_key{ TEXT("max") };
		static const FString low_key{ TEXT("low") };
		static const FString high_key{ TEXT("high") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}

		if (obj.HasField(min_key)) {
			auto src = obj.GetNumberField(min_key);
			task.set_min(src);
		}

		if (obj.HasField(max_key)) {
			auto src = obj.GetNumberField(max_key);
			task.set_max(src);
		}

		if (obj.HasField(low_key)) {
			auto src = obj.GetNumberField(low_key);
			task.set_low(src);
		}

		if (obj.HasField(high_key)) {
			auto src = obj.GetNumberField(high_key);
			task.set_high(src);
		}
	}
};

template <> struct config<FJsonObject, multiply> {
	template <typename Callback>
	void operator()(multiply &task, const FJsonObject &obj, Callback &callback) const
	{
		config<FJsonObject, accumulator<multiply>>{}(task, obj, callback);
	}
};

template <typename Noise> struct config<FJsonObject, perlin<Noise>> {
	template <typename Callback>
	void operator()(perlin<Noise> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto config = task.config();
		config<FJsonObject, noise_config>{}(config, obj, callback);
		task.set_config(config);
	}
};

template <typename Noise> struct config<FJsonObject, ridged_multifractal<Noise>> {
	template <typename Callback>
	void operator()(ridged_multifractal<Noise> &task, const FJsonObject &obj,
			Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto config = task.config();
		config<FJsonObject, ridged_multi_config>{}(config, obj, callback);
		task.set_config(config);
	}
};

template <> struct config<FJsonObject, scale_bias> {
	template <typename Callback>
	void operator()(scale_bias &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString scale_key{ TEXT("scale") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}

		if (obj.HasField(scale_key)) {
			auto src = callback(*obj.GetField(scale_key));
			task.set_scale(src);
		}

		if (obj.HasField(bias_key)) {
			auto src = callback(*obj.GetField(bias_key));
			task.set_bias(src);
		}
	}
};

template <> struct config<FJsonObject, scale_domain> {
	template <typename Callback>
	void operator()(scale_domain &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString scale_key{ TEXT("scale") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}

		for (auto i = 0; i < defaults::scale_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{UTF8_TO_TCHAR(math::to_c_str(v))};

			if (obj.HasField(var)) {
				auto src = callback(*obj.GetField(var));
				task.set_scale(v, src);
			}
		}
	}
};

template <typename Blender> struct config<FJsonObject, selector<Blender>> {
	template <typename Callback>
	void operator()(selector<Blender> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString switch_key{ TEXT("switch") };
		static const FString low_key{ TEXT("low") };
		static const FString high_key{ TEXT("high") };
		static const FString threshold_key{ TEXT("threshold") };
		static const FString falloff_key{ TEXT("falloff") };

		if (obj.HasField(switch_key)) {
			auto src = callback(*obj.GetField(switch_key));
			task.set_switch(src);
		}

		if (obj.HasField(low_key)) {
			auto src = callback(*obj.GetField(low_key));
			task.set_low(src);
		}

		if (obj.HasField(high_key)) {
			auto src = callback(*obj.GetField(high_key));
			task.set_high(src);
		}

		if (obj.HasField(threshold_key)) {
			auto src = callback(*obj.GetField(threshold_key));
			task.set_threshold(src);
		}

		if (obj.HasField(falloff_key)) {
			auto src = callback(*obj.GetField(falloff_key));
			task.set_falloff(src);
		}
	}
};

template <> struct config<FJsonObject, translate_domain> {
	template <typename Callback>
	void operator()(translate_domain &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}

		for (auto i = 0; i < defaults::translate_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{ UTF8_TO_TCHAR(math::to_c_str(v)) };

			if (obj.HasField(var)) {
				auto src = callback(*obj.GetField(var));
				task.set_translation(v, src);
			}
		}
	}
};

template <> struct config<FJsonObject, turbulence> {
	template <typename Callback>
	void operator()(turbulence &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString multiplier_key{ TEXT("multiplier") };


		if (obj.HasField(source_key)) {
			auto src = callback(*obj.GetField(source_key));
			task.set_source(src);
		}

		if (obj.HasField(multiplier_key)) {
			auto src = callback.eval(*obj.GetField(multiplier_key));
			task.set_multiplier(src);
		}

		for (auto i = 0; i < defaults::turbulence_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{ UTF8_TO_TCHAR(math::to_c_str(v)) };

			if (obj.HasField(var)) {
				auto src = callback(*obj.GetField(var));
				task.set_translation(v, src);
			}
		}
	}
};

} // namespace task
} // namespace tc

#endif // GRADIENT_NOISE_SRC_JSON_CONFIG
