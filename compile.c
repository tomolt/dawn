cover_reg()
{
}

cover_immed()
{
}

cover_compound()
{
}

cover_operand()
{
}

cover_modrm()
{
	cover_reg(lhs) and cover_operand(rhs);
	else
	cover_reg(rhs) and cover_operand(lhs);
}

cover_ins()
{
	switch () {
	case unop:
		cover_operand(arg);
		break;

	case binop:
		cover_modrm(lhs, rhs);
		break;
	}
}

/* Sethi-Ullman algorithm */

sort_()
{
}

su_()
{
	sort_();
	for (;;) {
	}
}

