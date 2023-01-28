#include <ncurses.h>
#include <string>
#include <list>

int main()
{
	int 								ch;
	std::string							buff;
	std::list<std::string>				str_list;
	std::list<std::string>::iterator	begin;
	std::list<std::string>::iterator	end;
	std::list<std::string>::iterator	curr;
	WINDOW 								*win;
	int									pos[4];
	int									bound[2];

/* Curses Initialisations */
	win = initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	printw("Welcome - Press q to Exit");
	printw("\n>> ");
	getyx(win, bound[1], bound[0]);
	while((ch = getch()) != 'q')
	{
		switch(ch)
		{
			case KEY_UP: {
				buff.clear();
				mvdelch(bound[1], bound[0]);
				if (!str_list.empty()) {
					printw((*curr).c_str());
					buff = *curr;
					if (curr != begin) --curr;
					else curr = end;
				}
				else printw("");
				getyx(win, pos[1], pos[0]);
				if (!pos[0]) {
					getmaxyx(win, pos[3], pos[2]);
					pos[3] = pos[1] - 1;
				} else {
					pos[3] = pos[1]; pos[2] = pos[0] - 1;
				}
				break ;
			}
			case KEY_DOWN: {
				//if (str_list.empty() || curr == end) break ;
				break ;
			}
			case KEY_LEFT:
				break;
			case KEY_RIGHT:
				break;
			case 127: {
				if (pos[0] <= bound[0] && pos[1] <= bound[1]) break ;
				mvdelch(pos[3], pos[2]);
				pos[0] = pos[2]; pos[1] = pos[3];
				if (!pos[0]) {
					getmaxyx(win, pos[3], pos[2]);
					pos[3] = pos[1] - 1;
				} else {
					pos[3] = pos[1]; pos[2] = pos[0] - 1;
				}
				if (!buff.empty()) buff.pop_back();
				break ;
			}
			case '\n': {
				if (!buff.empty()) str_list.push_back(buff);
				if (str_list.size() == 1) {
					begin = str_list.begin();
					end = begin;
				}
				if (!str_list.empty()) {
					end = str_list.end()--;
					curr = end;
				}
				printw("\n>> ");
				getyx(win, bound[1], bound[0]);
				buff.clear();
				break ;
			}
			default:
			{
				//printw("\nThe pressed key is ");
				//attron(A_BOLD);
				/*
				 pos[0] - current x
				 pos[1] - current y
				 pos[2] - previous x
				 pos[3] - previous y
				 * */
				getyx(win, pos[3], pos[2]);
				printw("%c", ch);
				getyx(win, pos[1], pos[0]);
				buff.push_back(ch);
				//attroff(A_BOLD);
			}
		}
	}

	printw("\n\nBye Now!\n");
	while (true) {
		if (str_list.empty()) break ;
		printw((*begin).c_str());
		printw("\n");
		++begin;
		if (begin == end) break ;
	}
	refresh();
	getch();
	endwin();

	return 0;
}
