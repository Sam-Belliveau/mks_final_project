#include "shell.h"

/**
 * @return a string that represents the home directory of the current user
 */
static const char* shell_get_home()
{
    static const char* home_dir = NULL;

    if(home_dir == NULL) home_dir = getpwuid(getuid())->pw_dir;

    return home_dir;
}

/**
 * @brief Display a prompt for the user to type into and reads data
 * 
 * this prompt includes the current directory, 
 * which is simplified if it is in the home directory
 * along with the user and the name of the shell.
 * 
 * @return the input that the user has typed
 */
struct shell_command* shell_readline()
{
    // create really large buffers
    char cwd[SH_CWD_SIZE] = {};
    char usr[SH_USR_SIZE] = {};
    char line[SH_USER_INPUT_BUFFER + 1] = {};

    // get information about home directory
    const char* home_dir = shell_get_home();
    int home_dir_len = strlen(home_dir);

    // copy cwd and hostname into buffers
    if(getcwd(cwd, SH_CWD_SIZE));
    gethostname(usr, SH_USR_SIZE);

    // build prompt for user
    fprintf(stderr, SH_COLOR_RESET "\n─────╮ " SH_COLOR_RED SH_PROGRAM_NAME SH_COLOR_RESET " : " SH_COLOR_GREEN "%s", usr);
    if(strncmp(cwd, home_dir, home_dir_len) == 0)
         fprintf(stderr, SH_COLOR_RESET "\n ╭───╯ " SH_COLOR_BLUE "~%s", cwd + home_dir_len);
    else fprintf(stderr, SH_COLOR_RESET "\n ╭───╯ " SH_COLOR_BLUE "%s", cwd);
    fprintf(stderr, SH_COLOR_RESET "\n─╯ ");

    // read input from user
    fgets(line, SH_USER_INPUT_BUFFER, stdin);

    // return command created from line
    return shell_command_create(line);
}

/**
 * @brief execute all of the commands in the shell_command linked list
 * 
 * the execution is done with shell_execute(...) which deals with every special case. 
 * 
 * @param command list of commands to execute
 */
void shell_execute_commands(struct shell_command* command)
{
    while(command != NULL)
    {
        shell_execute(command);
        command = command->next_command;
    }
}

/**
 * @brief execute an individual command.
 * 
 * this function will detect:
 *  - if the command is NULL
 *  - if the command has no arguments
 *  - if the command is "cd"
 *      - the command will then change the directory of the shell
 *  - if the command is "exit" / "quit"
 *      - the command will then close the shell
 * 
 * otherwise, the command will:
 *  1) set stdin, stdout, stderr to the commands specifications
 *  2) fork()
 *      2a) execvp()
 *      2b) waitpid()
 *  3) move the file descriptors back
 * 
 * @param command command to execute
 */
void shell_execute(struct shell_command* command)
{
    char dir[2 * SH_CWD_SIZE + 2] = {};
    int t_stdin, t_stdout, t_stderr;
    int status, f;

    // Throw out empty commands
    if(command == NULL) return;
    if(command->argc == 0) return;

    // Handle CD
    if(strcmp(command->argv[0], "cd") == 0)
    {
        if(command->argc != 2)
        {
            fprintf(stderr, SH_PROGRAM_NAME ": cd: 1 argument required, %d given\n", command->argc - 1);
        } else
        {
            if(strlen(command->argv[1]) >= SH_CWD_SIZE)
            { fprintf(stderr, SH_PROGRAM_NAME ": cd: length of given directory is too long [SH_CWD_SIZE=%d] \n", SH_CWD_SIZE); }

            else
            {
                // Check to see if the first character is '~'
                // if it is, then add the home directory to the beginning
                if(command->argv[1][0] == '~') sprintf(dir, "%.*s%.*s", SH_CWD_SIZE, shell_get_home(), SH_CWD_SIZE, command->argv[1] + 1);
                else sprintf(dir, "%.*s", SH_CWD_SIZE, command->argv[1]);

                // change directories
                status = chdir(dir);

                // print out error if cd fails
                if(status) fprintf(stderr, SH_PROGRAM_NAME ": cd: %s [%d]\n", strerror(errno), errno);
            }
        }
    }

    // Handle quit
    else if(
        strcmp(command->argv[0], "quit") == 0 ||
        strcmp(command->argv[0], "exit") == 0
    ) {
        exit(0);
    }

    // If no special command is entered
    // fork and run process
    else
    {
        // Add redirects to the command
        command = shell_command_add_redirects(command);

        // Make copies of standard fds
        t_stdin  = dup(SH_STDIN);
        t_stdout = dup(SH_STDOUT);
        t_stderr = dup(SH_STDERR);

        // Pipe outputs to the commands specified outputs
        dup2(command->redir_stdin,  SH_STDIN);
        dup2(command->redir_stdout, SH_STDOUT);
        dup2(command->redir_stderr, SH_STDERR);

        // Fork Process
        f = fork();
            
        // Child
        if(f == 0) 
        {
            status = execvp(command->argv[0], command->argv);
            
            // Handle different return values from child
            switch(errno)
            {
                // Returned Correctly, no error
                case 0: break;

                // "No such file or directory" = Command doesn't exist
                case 2:
                    fprintf(stderr, SH_PROGRAM_NAME ": command not found: %s\n", command->argv[0]);
                    break;
                
                // Handle every other error
                default:
                    fprintf(stderr, SH_PROGRAM_NAME ": %s [%d]\n", strerror(errno), errno);
                    break;
            }

            exit(status);
        }

        // Have parent wait for child
        else 
        {
            waitpid(f, &status, 0);
            status = WEXITSTATUS(status);
        }

        // Close all of the outputs opened by the command
        close(SH_STDIN);  safe_close(command->redir_stdin, SH_STDIN);
        close(SH_STDOUT); safe_close(command->redir_stdout, SH_STDOUT);
        close(SH_STDERR); safe_close(command->redir_stderr, SH_STDERR);
    
        // Move the outputs back to their place
        dup2(t_stdin,  SH_STDIN);  close(t_stdin);
        dup2(t_stdout, SH_STDOUT); close(t_stdout);
        dup2(t_stderr, SH_STDERR); close(t_stderr);
    }
}