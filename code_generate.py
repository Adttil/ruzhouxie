def generate_sequence(n, generate_once, split = ", "):
    if n == 0: 
        return ""
    result = generate_once(0)
    for i in range(1, n): 
        result += split + generate_once(i)
    return result

def generate_tuple_specialization(i):
    result = "template<" + generate_sequence(i, lambda i : "class T" + str(i)) + ">\n"
    result += "struct tuple<" + generate_sequence(i, lambda i : "T" + str(i)) + ">\n{\n"

    result += generate_sequence(i, lambda i : "    RUZHOUXIE(no_unique_address) T" + str(i) + " e" + str(i) + ";\n" , "")
    result += '\n'

    result += "    template<size_t I, derived_from<tuple> Self> requires (I < " + str(i) + "uz)\n"
    result += "    friend constexpr auto&& get(Self&& self) noexcept\n    {\n        "
    result += generate_sequence(i, lambda i : "if constexpr(I == " + str(i) + "uz) return FWD(self, " + "e" + str(i) + ");", "\n        else ")
    result += "\n    }\n\n"

    result += "    friend constexpr bool operator==(const tuple&, const tuple&) = default;\n"

    result += "};\n"
    return result
    
def generate_tuple_specialization_implement(max_element_count):
    result = ""
    for i in range(max_element_count + 1): 
        result += generate_tuple_specialization(i)
        result += '\n'
    return result

def generate_aggregate_getter_invoker_for(memeber_count):
    result = "if constexpr (n == " + str(memeber_count) + "uz)\n{\n"
	
    result += "    auto&& ["
    result += generate_sequence(memeber_count, lambda i : "m" + str(i))
    result += "] = FWD(t);\n"
	
    result += "    return ::rzx::arg_at<I>(";
    result += generate_sequence(memeber_count, lambda i : "FWDLIKE(t, m" + str(i) + ')')

    result += ");\n}\n"
    return result

def generate_aggregate_getter_invoker(max_memeber_count):
    return generate_sequence(max_memeber_count, lambda i : generate_aggregate_getter_invoker_for(i + 1), "else ")


with open("include/ruzhouxie/code_generate/tuple_specialization.code", 'w') as file:
    file.write(generate_tuple_specialization_implement(16))

with open("include/ruzhouxie/code_generate/aggregate_getter_invoker.code", 'w') as file:
    file.write(generate_aggregate_getter_invoker(64))