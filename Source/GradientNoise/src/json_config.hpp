#ifndef GRADIENT_NOISE_SRC_JSON_CONFIG_HPP
#define GRADIENT_NOISE_SRC_JSON_CONFIG_HPP

#include "CoreMinimal.h"
#include "Json.h"
#include "uproar/tasks.hpp"
#include "uproar/tasks/parse.hpp"
#include <string>

namespace tc
{
namespace task
{
template <> struct parse_callback<FJsonValue> {
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

template <> struct parse<FJsonObject, task_details> {
	void operator()(task_details &task, const FJsonObject &obj) const
	{
		static const FString name_key{ TEXT("name") };
		static const FString type_key{ TEXT("type") };
		static const FString rendered_key{ TEXT("rendered") };

		auto ue_to_std = [](const FString& str) {
			return std::string{ TCHAR_TO_UTF8(*str) };
		};

		if (obj.HasField(name_key)) {
			task.name =  ue_to_std(obj.GetStringField(name_key));
		}

		if (obj.HasField(type_key)) {
			task.type = ue_to_std(obj.GetStringField(type_key));
		}

		if (obj.HasField(rendered_key)) {
			task.rendered = obj.GetBoolField(rendered_key);
		}
	}
};

template <> struct parse<FJsonObject, noise_config> {
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

template <> struct parse<FJsonObject, ridged_multi_config> {
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

		parse<FJsonObject, noise_config>{}(task, obj, callback);
	}
};

template <typename Type> struct parse<FJsonObject, accumulator<Type>> {
	template <typename Callback>
	void operator()(accumulator<Type> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source_") };

		for (size_t i = 0; i < task.size(); ++i) {
			FString key = source_key + FString::FromInt(i);

			if (obj.HasField(key)) {
				auto src = callback(*obj.Values[key]);
				task.set_source(i, src);
			}
		}
	}
};

template <> struct parse<FJsonObject, bias_task> {
	template <typename Callback>
	void operator()(bias_task &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.source(src);
		}

		if (obj.HasField(bias_key)) {
			auto src = callback(*obj.Values[bias_key]);
			task.bias(src);
		}
	}
};

template <typename Noise> struct parse<FJsonObject, billowing<Noise>> {
	template <typename Callback>
	void operator()(billowing<Noise> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto c = task.config();
		parse<FJsonObject, noise_config>{}(c, obj, callback);
		task.set_config(c);
	}
};

template <> struct parse<FJsonObject, cache> {
	template <typename Callback>
	void operator()(cache &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.set_source(src);
		}
	}
};

template <> struct parse<FJsonObject, constant> {
	template <typename Callback>
	void operator()(constant &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString value_key{ TEXT("value") };

		if (obj.HasField(value_key)) {
			auto src = obj.GetNumberField(value_key);
			task.set_value(src);
		}
	}
};

template <> struct parse<FJsonObject, gradient> {
	template <typename Callback>
	void operator()(gradient &task, const FJsonObject &obj, Callback &callback) const
	{
		std::array<decimal_t, defaults::gradient_max_dimensions> l{};
		std::array<decimal_t, defaults::gradient_max_dimensions> r{};
		l.fill(0.0);
		r.fill(0.0);

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

template <> struct parse<FJsonObject, map_range> {
	template <typename Callback>
	void operator()(map_range &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString min_key{ TEXT("min") };
		static const FString max_key{ TEXT("max") };
		static const FString low_key{ TEXT("low") };
		static const FString high_key{ TEXT("high") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
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

template <> struct parse<FJsonObject, additive> {
	template <typename Callback>
	void operator()(additive &task, const FJsonObject &obj, Callback &callback) const
	{
		parse<FJsonObject, accumulator<additive>>{}(task, obj, callback);
	}
};

template <> struct parse<FJsonObject, multiply> {
	template <typename Callback>
	void operator()(multiply &task, const FJsonObject &obj, Callback &callback) const
	{
		parse<FJsonObject, accumulator<multiply>>{}(task, obj, callback);
	}
};

template <typename Noise> struct parse<FJsonObject, perlin<Noise>> {
	template <typename Callback>
	void operator()(perlin<Noise> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto c = task.config();
		parse<FJsonObject, noise_config>{}(c, obj, callback);
		task.set_config(c);
	}
};

template <typename Noise> struct parse<FJsonObject, ridged_multifractal<Noise>> {
	template <typename Callback>
	void operator()(ridged_multifractal<Noise> &task, const FJsonObject &obj,
			Callback &callback) const
	{
		static const FString seed_key{ TEXT("seed") };

		if (obj.HasField(seed_key)) {
			uint32_t seed = obj.GetIntegerField(seed_key);
			task.set_seed(seed);
		}

		auto c = task.config();
		parse<FJsonObject, ridged_multi_config>{}(c, obj, callback);
		task.set_config(c);
	}
};

template <> struct parse<FJsonObject, scale_bias> {
	template <typename Callback>
	void operator()(scale_bias &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString scale_key{ TEXT("scale") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.set_source(src);
		}

		if (obj.HasField(scale_key)) {
			auto src = callback(*obj.Values[scale_key]);
			task.set_scale(src);
		}

		if (obj.HasField(bias_key)) {
			auto src = callback(*obj.Values[bias_key]);
			task.set_bias(src);
		}
	}
};

template <> struct parse<FJsonObject, scale_domain> {
	template <typename Callback>
	void operator()(scale_domain &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString scale_key{ TEXT("scale") };
		static const FString bias_key{ TEXT("bias") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.set_source(src);
		}

		for (auto i = 0; i < defaults::scale_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{UTF8_TO_TCHAR(math::to_c_str(v))};

			if (obj.HasField(var)) {
				auto src = callback(*obj.Values[var]);
				task.set_scale(v, src);
			}
		}
	}
};

template <typename Blender> struct parse<FJsonObject, selector<Blender>> {
	template <typename Callback>
	void operator()(selector<Blender> &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString switch_key{ TEXT("switch") };
		static const FString low_key{ TEXT("low") };
		static const FString high_key{ TEXT("high") };
		static const FString threshold_key{ TEXT("threshold") };
		static const FString falloff_key{ TEXT("falloff") };

		if (obj.HasField(switch_key)) {
			auto src = callback(*obj.Values[switch_key]);
			task.set_switch(src);
		}

		if (obj.HasField(low_key)) {
			auto src = callback(*obj.Values[low_key]);
			task.set_low(src);
		}

		if (obj.HasField(high_key)) {
			auto src = callback(*obj.Values[high_key]);
			task.set_high(src);
		}

		if (obj.HasField(threshold_key)) {
			auto src = callback(*obj.Values[threshold_key]);
			task.set_threshold(src);
		}

		if (obj.HasField(falloff_key)) {
			auto src = callback(*obj.Values[falloff_key]);
			task.set_falloff(src);
		}
	}
};

template <> struct parse<FJsonObject, translate_domain> {
	template <typename Callback>
	void operator()(translate_domain &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };

		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.set_source(src);
		}

		for (auto i = 0; i < defaults::translate_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{ UTF8_TO_TCHAR(math::to_c_str(v)) };

			if (obj.HasField(var)) {
				auto src = callback(*obj.Values[var]);
				task.set_translation(v, src);
			}
		}
	}
};

template <> struct parse<FJsonObject, turbulence> {
	template <typename Callback>
	void operator()(turbulence &task, const FJsonObject &obj, Callback &callback) const
	{
		static const FString source_key{ TEXT("source") };
		static const FString multiplier_key{ TEXT("multiplier") };


		if (obj.HasField(source_key)) {
			auto src = callback(*obj.Values[source_key]);
			task.set_source(src);
		}

		if (obj.HasField(multiplier_key)) {
			auto src = callback.eval(*obj.GetField<EJson::Object>(multiplier_key));
			task.set_multiplier(src);
		}

		for (auto i = 0; i < defaults::turbulence_max_sources; ++i) {
			auto v = static_cast<math::variable>(i);
			auto var = FString{ UTF8_TO_TCHAR(math::to_c_str(v)) };

			if (obj.HasField(var)) {
				auto src = callback(*obj.Values[var]);
				task.set_translation(v, src);
			}
		}
	}
};

} // namespace task
} // namespace tc

#endif // GRADIENT_NOISE_SRC_JSON_CONFIG_HPP
