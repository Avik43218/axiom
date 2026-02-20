class CCEConfig:

    from dotenv import load_dotenv
    import os

    load_dotenv()

    MYSQL_DB_NAME = "student_records"
    MYSQL_DB_PASSWD = os.getenv("DB_PASSWD")
    MYSQL_DB_HOST = "localhost"
    MYSQL_DB_USER = "root"
    