#include <stdio.h>
#include <sys/socket.h>         
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <errno.h>

#define PORT 8081

char* parse(char line[], const char symbol[]);
char* parse_method(char line[], const char symbol[]);
char* find_token(char line[], const char symbol[], const char match[]);
int send_message(int fd, char image_path[], char head[]);

char http_header[25] = "HTTP/1.1 200 Ok\r\n";


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, pid; 
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In sockets");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        //Create child process to handle request from different client
        pid = fork();
        if(pid < 0){
            perror("Error on fork");
            exit(EXIT_FAILURE);
        }
        
        if(pid == 0){
            char buffer[30000] = {0};
            valread = read( new_socket , buffer, 30000);

            printf("\n buffer message: %s \n ", buffer);
            char *parse_string_method = parse_method(buffer, " ");  //Try to get the path which the client ask for
            printf("Client method: %s\n", parse_string_method);

            char *parse_string = parse(buffer, " ");  //Try to get the path which the client ask for
            printf("Client ask for path: %s\n", parse_string);
            char *copy = (char *)malloc(strlen(parse_string) + 1);
            strcpy(copy, parse_string);
            char *parse_ext = parse(copy, ".");  // get the file extension such as JPG, jpg

            char *copy_head = (char *)malloc(strlen(http_header) +200);
            strcpy(copy_head, http_header);

            if(parse_string_method[0] == 'G' && parse_string_method[1] == 'E' && parse_string_method[2] == 'T'){
                if(strlen(parse_string) <= 1){
                    char path_head[500] = ".";
                    strcat(path_head, "/index.html");
                    strcat(copy_head, "Content-Type: text/html\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                else if ((parse_ext[0] == 'j' && parse_ext[1] == 'p' && parse_ext[2] == 'g') || (parse_ext[0] == 'J' && parse_ext[1] == 'P' && parse_ext[2] == 'G'))
                {
                    //send image to client
                    char path_head[500] = ".";
                    strcat(path_head, parse_string);
                    strcat(copy_head, "Content-Type: image/jpeg\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                else if (parse_ext[0] == 'i' && parse_ext[1] == 'c' && parse_ext[2] == 'o')
                {
                    char path_head[500] = ".";
                    strcat(path_head, "/img/favicon.png");
                    strcat(copy_head, "Content-Type: image/vnd.microsoft.icon\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                else if (parse_ext[0] == 't' && parse_ext[1] == 't' && parse_ext[2] == 'f')
                {
                    char path_head[500] = ".";
                    strcat(path_head, parse_string);
                    strcat(copy_head, "Content-Type: font/ttf\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                else if (parse_ext[strlen(parse_ext)-2] == 'j' && parse_ext[strlen(parse_ext)-1] == 's')
                {
                    //javascript
                    char path_head[500] = ".";
                    strcat(path_head, parse_string);
                    strcat(copy_head, "Content-Type: text/javascript\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                else if (parse_ext[strlen(parse_ext)-3] == 'c' && parse_ext[strlen(parse_ext)-2] == 's' && parse_ext[strlen(parse_ext)-1] == 's')
                {
                    //css
                    char path_head[500] = ".";
                    strcat(path_head, parse_string);
                    strcat(copy_head, "Content-Type: text/css\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                }
                // else if (parse_ext[0] == 'w' && parse_ext[1] == 'o' && parse_ext[2] == 'f')
                // {
                //     //Web Open Font Format woff and woff2
                //     char path_head[500] = ".";
                //     strcat(path_head, parse_string);
                //     strcat(copy_head, "Content-Type: font/woff\r\n\r\n");
                //     send_message(new_socket, path_head, copy_head);
                // }
                // else if (parse_ext[0] == 'm' && parse_ext[1] == '3' && parse_ext[2] == 'u' && parse_ext[3] == '8')
                // {
                //     //Web Open m3u8
                //     char path_head[500] = ".";
                //     strcat(path_head, parse_string);
                //     strcat(copy_head, "Content-Type: application/vnd.apple.mpegurl\r\n\r\n");
                //     send_message(new_socket, path_head, copy_head);
                // }
                // else if (parse_ext[0] == 't' && parse_ext[1] == 's')
                // {
                //     //Web Open ts
                //     char path_head[500] = ".";
                //     strcat(path_head, parse_string);
                //     strcat(copy_head, "Content-Type: video/mp2t\r\n\r\n");
                //     send_message(new_socket, path_head, copy_head);
                // }
                else{
                    //send other file 
                    char path_head[500] = ".";
                    strcat(path_head, parse_string);
                    strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
                    send_message(new_socket, path_head, copy_head);
                    printf("Else: %s \n", parse_string);        
                }
                printf("\n------------------Server sent----------------------------------------------------\n");
            }
            else if (parse_string_method[0] == 'P' && parse_string_method[1] == 'O' && parse_string_method[2] == 'S' && parse_string_method[3] == 'T'){
                char *find_string = (char*)malloc(200);
                find_string = find_token(buffer, "\r\n", "action");
                strcat(copy_head, "Content-Type: text/plain \r\n\r\n");
                //strcat(copy_head, "Content-Length: 12 \n");
                strcat(copy_head, "User Action: ");
                printf("find string: %s \n", find_string);
                strcat(copy_head, find_string);
                write(new_socket, copy_head, strlen(copy_head));
            }
            close(new_socket);
            free(copy);
            free(copy_head);  
        }
        else{
            printf(">>>>>>>>>>Parent create child with pid: %d <<<<<<<<<", pid);
            close(new_socket);
        }
    }
    close(server_fd);
    return 0;
}

char* parse(char line[], const char symbol[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);
    
    char *message;
    char * token = strtok(copy, symbol);
    int current = 0;

    while( token != NULL ) {
      
      token = strtok(NULL, " ");
      if(current == 0){
          message = token;
          if(message == NULL){
              message = "";
          }
          return message;
      }
      current = current + 1;
   }
   free(token);
   free(copy);
   return message;
}

char* parse_method(char line[], const char symbol[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);
        
    char *message;
    char * token = strtok(copy, symbol);
    int current = 0;

    while( token != NULL ) {
      
      //token = strtok(NULL, " ");
      if(current == 0){
          message = token;
          if(message == NULL){
              message = "";
          }
          return message;
      }
      current = current + 1;
   }
   free(copy);
   free(token);
   return message;
}

char* find_token(char line[], const char symbol[], const char match[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);
        
    char *message;
    char * token = strtok(copy, symbol);

    while( token != NULL ) {
      
      //printf("--Token: %s \n", token);`
      
      if(strlen(match) <= strlen(token))
      {
          int match_char = 0;
          for(int i = 0; i < strlen(match); i++)
          {
              if(token[i] == match[i])
              {
                  match_char++;
              }
          }
          if(match_char == strlen(match)){
            message = token;
            return message;
          }
      }      
      token = strtok(NULL, symbol);
   }
   free(copy);
   free(token);
   message = "";
   return message;
}

int send_message(int fd, char image_path[], char head[]){

    struct stat stat_buf;

    write(fd, head, strlen(head));

    int fdimg = open(image_path, O_RDONLY);
    
    if(fdimg < 0){
        printf("Cannot Open file path : %s with error %d\n", image_path, fdimg); 
    }
     
    fstat(fdimg, &stat_buf);
    int img_total_size = stat_buf.st_size;
    int block_size = stat_buf.st_blksize;
    if(fdimg >= 0){
        ssize_t sent_size;

        while(img_total_size > 0){
              int send_bytes = ((img_total_size < block_size) ? img_total_size : block_size);
              int done_bytes = sendfile(fd, fdimg, NULL, block_size);
              img_total_size = img_total_size - done_bytes;
        }
        if(sent_size >= 0){
            printf("send file: %s \n" , image_path);
        }
        close(fdimg);
    }
}
