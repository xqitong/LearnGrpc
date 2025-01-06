using Google.Protobuf.WellKnownTypes;
using GrpcServer.Web.Protos;
using System.Data;

namespace GrpcServer.Web.Data
{
    public class InMemoryData
    {
        public static List<Employee> Employees = new List<Employee>
        {
            new Employee { Id = 1, 
                        No = 101,
                        FirstName = "Alice",
                        LastName = "Smith",
                        MonthSalary = new MonthSalary{ Basic = 5000f, Bonus = 125.5f },
                        Status = EmployeeStatus.Normal,
                        LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
                        /*, Salary = 60000*/ },
            new Employee { Id = 2, 
                        No = 102,
                        FirstName = "Bob", 
                        LastName = "Johnson",
                        MonthSalary = new MonthSalary{ Basic = 1000f, Bonus = 0f },
                        Status = EmployeeStatus.Retired,
                        LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
                        /*, Salary = 75000*/ },
            new Employee { Id = 3, 
                        No = 103, 
                        FirstName = "Charlie", 
                        LastName = "Brown",
                        MonthSalary = new MonthSalary{ Basic = 2000f, Bonus = 2f },
                        Status = EmployeeStatus.Resigned,
                        LastModified = Timestamp.FromDateTime(DateTime.UtcNow)/*, Salary = 50000*/ }
        };
    }
}
