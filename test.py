import enum
from subprocess import CompletedProcess, run, PIPE
from typing import List, Tuple
from xml.dom import NotFoundErr

PASS = "\033[38;5;154m[ OK ]\033[0m"
FAIL = "\033[38;5;196m[ FAIL ]\033[0m"

BLUE = "\033[38;5;12m"
BOLD = "\033[1m"
END = "\033[0m"

NOT_FOUND_MESSAGE = "Not found"
FOUND_MESSAGE = "Kontakt(y) nalezen(y)"

BASE_INPUT = [
    ("Petr Dvorak", "603123456"),
    ("Jana Novotna", "777987654"),
    ("Bedrich Smetana ml.", "541141120"),
]

NON_CONTIGUOUS_INPUT = [("XAXXXXXXBC", "123144312"), ("XXXXXABDXX", "478698143")]


class Tester:
    def __init__(self, program_name: str, first_bonus: bool = False) -> None:
        self.program_name = "./" + program_name
        self.test_count = 0
        self.pass_count = 0
        self.first_bonus = first_bonus

    def test(
        self,
        test_name: str,
        args: List[str],
        input_: List[Tuple[str, str]],
        expected_contacts: List[int],
        should_fail: bool = False,
        bonus_contacts: List[int] = None,
    ):
        self.test_count += 1
        failed = False
        error_msg: str = ""

        str_input = self.create_input(input_)
        if self.first_bonus and bonus_contacts is not None:
            str_output = self.create_output(
                input_, list(set(expected_contacts + bonus_contacts))
            )
        else:
            str_output = self.create_output(input_, expected_contacts)

        p: CompletedProcess[str]

        try:
            p = run(
                [self.program_name] + args,
                stdout=PIPE,
                stderr=PIPE,
                input=str_input,
                encoding="ascii",
            )
        except UnicodeEncodeError as e:
            self.print_fail(test_name)
            print("Vystup obsahuje znaky ktere nepatri do ASCII (napr. diakritika)")
            print(e)
        except Exception as e:
            self.print_fail(test_name)
            print("Chyba pri volani programu")
            print(e)
            exit(1)

        if p.returncode != 0:
            if not should_fail:
                failed = True
                error_msg += f"Program vratil chybovy navratovy kod {p.returncode} prestoze nemel\n"

        else:
            if should_fail:
                failed = True
                error_msg += "Program vratil byl uspesne ukoncen, i presto ze nemel\n"

        if not self.assert_equal(str_output, p.stdout):
            failed = True
            error_msg += "Vystup programu se neshoduje s ocekavanym vystupem"

        if should_fail and len(p.stderr) == 0: 
            failed = True
            error_msg += "Program nevratil chybovou hlasku na STDERR\n"

        if failed:
            self.print_fail(test_name)
            print(error_msg)
            print(f"{self.bold('Argumenty')}: {' '.join(args)}")
            print(f"{self.bold('Predpokladany vystup')}:")
            print(self.debug(str_output))
            print(f"{self.bold('STDOUT')}:")
            print(self.debug(p.stdout))
            print(f"{self.bold('STDERR')}:")
            print(self.debug(p.stderr))
        else:
            self.pass_count += 1
            self.print_pass(test_name)

    def print_stats(self):
        success_rate = self.pass_count / self.test_count * 100
        print(
            self.bold(f"Uspesnost: {success_rate:.2f} % [{self.pass_count} / {self.test_count}]")
        )

    def print_fail(self, msg: str) -> None:
        print(FAIL, msg)

    def print_pass(self, msg: str) -> None:
        print(PASS, msg)

    def assert_equal(self, output: str, expected_output: str) -> bool:
        lines = {line.lower() for line in expected_output.rstrip().split("\n")}

        for line in output.rstrip().split("\n"):
            line = line.lower()

            if line not in lines:
                return False

        return True

    def create_input(self, input_: List[Tuple[str, str]]) -> str:
        return "".join([f"{name}\n{number}\n" for name, number in input_])

    def create_output(
        self, input_: List[Tuple[str, str]], exptected_contacts: List[int]
    ) -> str:
        out = (FOUND_MESSAGE if len(exptected_contacts) else NOT_FOUND_MESSAGE) + "\n"
        for i, (name, number) in enumerate(input_):
            if i + 1 in exptected_contacts:
                out += f"{name.lower()}, {number.lower()}\n"

        return out

    def debug(self, text: str) -> str:
        return f"{BLUE}{text}{END}"

    def bold(self, text: str) -> str:
        return f"{BOLD}{text}{END}"


if __name__ == "__main__":
    first_bonus = True
    second_bonus = False 
    t = Tester("t9search", first_bonus)

    t.test("Test ze zadani #1", [], BASE_INPUT, [1, 2, 3], bonus_contacts=[])
    t.test("Test ze zadani #2", ["12"], BASE_INPUT, [1, 3], bonus_contacts=[])
    t.test("Test ze zadani #3", ["686"], BASE_INPUT, [2], bonus_contacts=[])
    t.test("Test ze zadani #4", ["38"], BASE_INPUT, [1, 3], bonus_contacts=[])
    t.test("Test ze zadani #5", ["111"], BASE_INPUT, [], bonus_contacts=[3])

    if first_bonus:
        t.test("Test na prvni rozsireni #1", ["222"], NON_CONTIGUOUS_INPUT, [1])
        t.test("Test na prvni rozsireni #2", ["111"], NON_CONTIGUOUS_INPUT, [1])
        t.test("Test na prvni rozsireni #3", ["223"], NON_CONTIGUOUS_INPUT, [2])
        t.test("Test na prvni rozsireni #4", ["892"], NON_CONTIGUOUS_INPUT, [])
    
    if second_bonus:
        pass

    t.print_stats()
