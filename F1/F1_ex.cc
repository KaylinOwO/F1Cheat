#include "stdafx.hh"

#include "F1_ex.hh"

#include "../SDK/SDK.hh"

#include <tier0/memdbgon.h>

std::string buffer;

void error(const char *w)
{
	printf("Error parsing: %s\n", w);
}

float GetSymbolValue(const char *&s)
{
	return 1.0f;
}

float CompileLiteral(const char *&s, bool eval)
{
	int v = 0;
	while (*s >= '0' && *s <= '9') {
		v = v * 10 + *s++ - '0';
	}
	// printf("    mov  eax, %i\n", v);
	if (!eval)
		buffer += std::to_string(v);
	return v;
}

float CompileSymbol(const char *&s, bool evaluate)
{
	if (!evaluate) {
		while ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') || (*s == '_')) {
			buffer += (*s++);
		}
		return 0.0f;
	} else {
		std::string temp;
		while ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') || (*s == '_')) {
			temp += (*s++);
		}
		const char *s = temp.c_str();
		return GetSymbolValue(s);
	}
}

float CompileExpression(const char *, bool);

float CompileTerm(const char *&s, bool eval = false)
{
	if (*s >= '0' && *s <= '9') {
		// Number
		return CompileLiteral(s, eval);
	} else if ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s == '_')) {
		// Variable
		return CompileSymbol(s, eval);
	} else if (*s == '-') {
		// Unary negation
		s++;
		if (!eval)
			buffer += '-';
		return CompileTerm(s, eval);
		// printf("    neg  eax\n");
	} else if (*s == '(') {
		// Parenthesized sub-expression
		s++;
		float f = CompileExpression(s, eval);
		if (*s != ')')
			error("')' expected");
		s++;
		return f;
	} else if (*s == '=') {
		s++; // consume
		buffer += " = ";
		float f = CompileExpression(s, true);
		if (!eval)
			buffer += std::to_string(f);
		return f;
	} else {
		error("Syntax error");
	}
}

float CompileMulDiv(const char *&s, bool eval)
{
	if (!eval) {
		CompileTerm(s, false);
		for (;;) {
			if (*s == '*') {
				s++;
				buffer += " * ";
				CompileTerm(s);
			} else if (*s == '/') {
				s++;
				buffer += " / ";
				CompileTerm(s);
			} else
				break;
		}
	} else {
		float f = CompileTerm(s, true);
		for (;;) {
			if (*s == '*') {
				s++;
				return f * CompileTerm(s, true);
			} else if (*s == '/') {
				s++;
				return f / CompileTerm(s, true);
			} else
				break;
		}
	}

	return 0.0f;
}

float CompileAddSub(const char *&s, bool eval)
{
	if (!eval) {
		CompileMulDiv(s, eval);
		for (;;) {
			if (*s == '+') {
				s++;
				buffer += " + ";
				CompileMulDiv(s, false);
			} else if (*s == '-') {
				s++;
				buffer += " - ";
				CompileMulDiv(s, false);
			} else
				break;
		}
	} else {
		float f = CompileMulDiv(s, true);
		for (;;) {
			if (*s == '+') {
				s++;
				return f + CompileMulDiv(s, true);
			} else if (*s == '-') {
				s++;
				return f - CompileMulDiv(s, true);
			} else
				break;
		}
	}

	return 0.0f;
}

float CompileExpression(const char *s, bool eval)
{
	float f = CompileAddSub(s, eval);
	if (eval)
		printf("Evaluated %s to %f\n", s, f);
	return f;
}

void F1_ParseEx(const char *&s)
{
	CompileExpression(s, false);
}

int main()
{
	const char *s = "v=3*3/-(-4+3)";
	F1_ParseEx(s);
	printf("%s", buffer.c_str());
}
