import random


class Generator:
    def __init__(
        self,
        int_lim=(-100, 100),
        exp_op=["-", "+"],
        term_op=["/", "*"],
        unary=["-"],
        max_recursion=100,
    ):
        self.recurse = 0
        self.recurse_max = max_recursion
        self.int_lim = int_lim
        self.expression_op = exp_op
        self.unary = unary
        self.term_op = term_op

    def generate_primary(self):
        if self.recurse > self.recurse_max:
            num = random.randint(*self.int_lim)
            return str(num)

        self.recurse += 1
        if random.randint(1, 6) <= 3:
            # Return a number
            num = random.randint(*self.int_lim)
            if random.randint(1, 2) == 1:
                return str(num)
            else:
                return str(-1 * num)
        else:
            return "(" + self.generate_expression() + ")"

    def generate_factor(self):
        self.recurse += 1
        return (
            self.generate_primary()
            + random.choice(self.term_op)
            + self.generate_primary()
        )

    def generate_term(self):
        self.recurse += 1
        # term = factor, {"*", "/", {"-"}factor}
        return (
            self.generate_factor() + random.choice(self.unary) + self.generate_factor()
        )

    def generate_expression(self):
        # expression = term, {{"+" | "-"}, term}
        self.recurse += 1
        return (
            self.generate_term()
            + random.choice(self.expression_op)
            + self.generate_term()
        )

    def generate(self):
        expr = self.generate_expression()
        self.recurse = 0
        return expr

    def set_recurse_max(self, val):
        self.recurse_max = val


def main():
    g = Generator()
    print(g.generate())


if __name__ == "__main__":
    main()
